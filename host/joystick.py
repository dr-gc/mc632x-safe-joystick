#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
双摇杆映射模块
==============
左手摇杆 / 右手摇杆各一套，都支持：
  · 独立的校准零点（连续握拳 3 次 -> 平均位置设为 0 点）
  · 工作区内相对零点偏移 -> 摇杆输出
  · 握拳 = 急停（严格判定，需四指全蜷 + 拇指收拢）
  · 手离开工作区 = 平滑回中
"""
import math
import time


class JoystickState:
    def __init__(self):
        self.active = False
        self.estop = False
        self.x = 0
        self.y = 0
        self.direction = "STOP"
        self.magnitude = 0
        self.in_zone = False
        self.gesture = "NONE"
        self.hand_present = False

    def as_dict(self):
        return {"active": self.active, "estop": self.estop,
                "x": self.x, "y": self.y, "direction": self.direction,
                "magnitude": self.magnitude, "in_zone": self.in_zone,
                "gesture": self.gesture, "hand_present": self.hand_present}


class Calibration:
    """校准状态机：收集 N 次握拳时的掌心位置，取平均作为零点"""

    def __init__(self, need=3):
        self.need = need
        self.reset()

    def reset(self):
        self.active = False
        self.samples = []
        self.done = False
        self.origin = None          # (x, y) 校准后的零点
        self._last_fist = False     # 用于检测握拳的上升沿

    def start(self):
        self.reset()
        self.active = True
        return self

    def feed(self, gest, palm):
        """gest: 手势名, palm: (px, py) 或 None
        返回: 当前已采集次数"""
        if not self.active:
            return len(self.samples)

        is_fist = (gest == "FIST")
        rising = is_fist and not self._last_fist
        self._last_fist = is_fist

        if rising and palm is not None:
            self.samples.append(palm)
            if len(self.samples) >= self.need:
                xs = [s[0] for s in self.samples]
                ys = [s[1] for s in self.samples]
                self.origin = (sum(xs) / len(xs), sum(ys) / len(ys))
                self.active = False
                self.done = True
        return len(self.samples)

    def as_dict(self):
        return {"active": self.active, "count": len(self.samples),
                "need": self.need, "done": self.done,
                "origin": list(self.origin) if self.origin else None}


class HandJoystick:
    """单手摇杆"""

    def __init__(self, name, zone, dead_zone=0.08, smoothing=0.4,
                 estop_gesture="OPEN_PALM"):
        self.name = name
        self.zone = zone
        self.dead_zone = dead_zone
        self.smoothing = smoothing
        self.estop_gesture = estop_gesture
        self.state = JoystickState()
        self.cal = Calibration(3)
        self._sx = 0.0
        self._sy = 0.0
        self.gesture = "NONE"

    # ---------- 几何 ----------
    def zone_center(self):
        x1, y1, x2, y2 = self.zone
        return (x1 + x2) * 0.5, (y1 + y2) * 0.5

    def zone_size(self):
        x1, y1, x2, y2 = self.zone
        return x2 - x1, y2 - y1

    def point_in_zone(self, px, py):
        x1, y1, x2, y2 = self.zone
        return x1 <= px <= x2 and y1 <= py <= y2

    def effective_center(self):
        """有校准零点则用它，否则用工作区几何中心"""
        if self.cal.origin:
            return self.cal.origin
        return self.zone_center()

    # ---------- 主更新 ----------
    def update(self, hand, gest):
        st = self.state
        st.gesture = gest

        # ── 取手心位置
        palm = None
        if hand is not None:
            lm = hand["landmarks"]
            palm = ((lm[0][0] + lm[9][0]) * 0.5, (lm[0][1] + lm[9][1]) * 0.5)

        st.hand_present = palm is not None

        # ── 校准模式：收集握拳样本
        if self.cal.active:
            n = self.cal.feed(gest, palm)
            st.estop = False
            st.active = False
            st.in_zone = palm is not None and self.point_in_zone(*palm)
            st.x = st.y = 0
            st.direction = "CALIB"
            st.magnitude = 0
            self._sx = self._sy = 0.0
            return st

        # ── 急停优先
        if gest == self.estop_gesture:
            st.estop = True
            st.active = False
            st.in_zone = False
            self._sx = self._sy = 0.0
            st.x = st.y = 0
            st.direction = "ESTOP"
            st.magnitude = 0
            return st
        st.estop = False

        # ── 无手 -> 回中
        if palm is None:
            self._sx *= (1 - self.smoothing)
            self._sy *= (1 - self.smoothing)
            if abs(self._sx) < 20 and abs(self._sy) < 20:
                self._sx = self._sy = 0.0
            st.x = int(self._sx)
            st.y = int(self._sy)
            st.in_zone = False
            st.active = False
            st.direction = "STOP"
            st.magnitude = 0
            return st

        px, py = palm
        in_zone = self.point_in_zone(px, py)
        st.in_zone = in_zone

        if not in_zone:
            self._sx *= (1 - self.smoothing)
            self._sy *= (1 - self.smoothing)
            st.x = int(self._sx)
            st.y = int(self._sy)
            st.active = False
            st.direction = "STOP"
            st.magnitude = 0
            return st

        # ── 相对零点偏移
        cx, cy = self.effective_center()
        zw, zh = self.zone_size()
        half_w = max(zw * 0.5, 1e-6)
        half_h = max(zh * 0.5, 1e-6)

        nx = (px - cx) / half_w
        ny = -(py - cy) / half_h
        nx = max(-1.0, min(1.0, nx))
        ny = max(-1.0, min(1.0, ny))

        dist = math.hypot(nx, ny)
        if dist < self.dead_zone:
            tx_, ty_ = 0.0, 0.0
            st.direction = "STOP"
        else:
            scale = (dist - self.dead_zone) / max(1.0 - self.dead_zone, 1e-6)
            ux, uy = nx / dist, ny / dist
            tx_, ty_ = ux * scale * 1000.0, uy * scale * 1000.0
            st.direction = self._direction_of(nx, ny)

        a = self.smoothing
        self._sx += (tx_ - self._sx) * a
        self._sy += (ty_ - self._sy) * a

        st.x = int(max(-1000, min(1000, self._sx)))
        st.y = int(max(-1000, min(1000, self._sy)))
        st.magnitude = int(min(1000, math.hypot(st.x, st.y)))
        st.active = st.magnitude > 12
        return st

    @staticmethod
    def _direction_of(nx, ny):
        """八方向判定（平面分成 8 个 45° 扇区）

        以 +Y 为前、+X 为右：
          扇区中心角   方向
           0°(正前)   FWD
          45°         FWD_RIGHT
          90°(正右)   RIGHT
         135°         BACK_RIGHT
         180°(正后)   BACK
         225°         BACK_LEFT
         270°(正左)   LEFT
         315°         FWD_LEFT
        """
        import math
        mag = math.hypot(nx, ny)
        if mag < 1e-6:
            return "STOP"

        # atan2(nx, ny) 给出相对 +Y 的角度，顺时针为正，范围 -180..180
        ang = math.degrees(math.atan2(nx, ny))
        if ang < 0:
            ang += 360.0

        # 每 45° 一个扇区，±22.5° 边界
        sector = int((ang + 22.5) // 45) % 8
        return ["FWD", "FWD_RIGHT", "RIGHT", "BACK_RIGHT",
                "BACK", "BACK_LEFT", "LEFT", "FWD_LEFT"][sector]

    def as_dict(self):
        d = self.state.as_dict()
        d["cal"] = self.cal.as_dict()
        d["center"] = list(self.effective_center())
        d["zone"] = list(self.zone)
        return d


class DualJoystick:
    """双手摇杆：左半屏 -> 左手，右半屏 -> 右手"""

    def __init__(self):
        self.left = HandJoystick("L", zone=(0.0, 0.0, 0.5, 1.0))   # 保留，未使用
        self.right = HandJoystick("R", zone=(0.0, 0.0, 1.0, 1.0))  # 单摇杆：整个手势画面

    @staticmethod
    def _pick_hand(hands):
        """按 handedness 分配，返回 (left_hand, right_hand)"""
        L = R = None
        for h in hands:
            name = (h.get("handedness") or "").lower()
            if name in ("left", "左手"):
                if L is None:
                    L = h
            elif name in ("right", "右手"):
                if R is None:
                    R = h
        # 未标注时按掌心 x 坐标分配
        rest = [h for h in hands if h is not L and h is not R]
        for h in rest:
            lm = h["landmarks"]
            cx = (lm[0][0] + lm[9][0]) * 0.5
            if cx < 0.5 and L is None:
                L = h
            elif R is None:
                R = h
        return L, R

    def update(self, hands, gestures):
        """单摇杆模式：取第一只手（或右手优先），全部交给 self.right"""
        hands = hands or []
        gestures = gestures or []

        # 优先选右手，没有右手就用第一只
        R = None
        gr = "NONE"
        for i, h in enumerate(hands):
            name = (h.get("handedness") or "").lower()
            if name == "right":
                R = h
                gr = gestures[i] if i < len(gestures) else "NONE"
                break
        if R is None and hands:
            R = hands[0]
            gr = gestures[0] if gestures else "NONE"

        # 左手位保留为空（单摇杆不使用）
        self.left.update(None, "NONE")
        self.right.update(R, gr)

    def start_calibration(self, which="both"):
        if which in ("both", "left"):
            self.left.cal.start()
        if which in ("both", "right"):
            self.right.cal.start()

    def reset_calibration(self, which="both"):
        if which in ("both", "left"):
            self.left.cal.reset()
            self.left.cal.origin = None
            self.left._sx = self.left._sy = 0.0
        if which in ("both", "right"):
            self.right.cal.reset()
            self.right.cal.origin = None
            self.right._sx = self.right._sy = 0.0

    def as_dict(self):
        return {"left": self.left.as_dict(), "right": self.right.as_dict()}
