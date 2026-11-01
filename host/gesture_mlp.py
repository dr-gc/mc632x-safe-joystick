#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
手势分类器（基于 Kazuhito00/hand-gesture-recognition-using-mediapipe 的预训练 MLP）
=============================================================================
模型：models/keypoint_classifier.tflite（6.1KB，MLP）
类别：Open / Close / Pointer
来源：https://github.com/Kazuhito00/hand-gesture-recognition-using-mediapipe
授权：Apache-2.0

为什么要用 MLP 而不是手写角度阈值：
  - 预处理把关键点做「手腕为原点 + 最大值归一化」，消除了位置/尺度/朝向影响
  - MLP 从真实标注数据里学「Close（握拳）」的形态，半张手在训练集里见过，不会误判
  - 手写规则只能用几个阈值近似，边界上必然误判
"""
import itertools
import os

import numpy as np

try:
    from ai_edge_litert.interpreter import Interpreter as _Interpreter
except ImportError:                                   # 老版本回退
    try:
        from tflite_runtime.interpreter import Interpreter as _Interpreter
    except ImportError:
        _Interpreter = None

_HERE = os.path.dirname(os.path.abspath(__file__))
MODEL_PATH = os.path.join(_HERE, "models", "keypoint_classifier.tflite")
LABEL_PATH = os.path.join(_HERE, "models", "keypoint_classifier_label.csv")

# 模型输出的类别索引 -> 我们的手势名
# 0=Open 1=Close 2=Pointer
IDX_TO_GESTURE = {0: "OPEN_PALM", 1: "FIST", 2: "POINTER"}


def pre_process_landmark(landmark_list):
    """关键点预处理（复刻原仓库实现）

    landmark_list: [[x, y], ...] 21 点，归一化到 0~1
    返回：42 维扁平列表，已消除平移和尺度影响
    """
    tmp = [[float(p[0]), float(p[1])] for p in landmark_list]

    # 1) 以手腕(0 号点)为原点
    base_x, base_y = tmp[0][0], tmp[0][1]
    for p in tmp:
        p[0] -= base_x
        p[1] -= base_y

    # 2) 拉平
    flat = list(itertools.chain.from_iterable(tmp))

    # 3) 按绝对值最大值归一化到 [-1, 1]
    max_value = max(abs(v) for v in flat) or 1.0
    return [v / max_value for v in flat]


class GestureClassifier:
    """MLP 手势分类器"""

    def __init__(self, model_path=MODEL_PATH):
        if _Interpreter is None:
            raise RuntimeError(
                "缺少 TFLite 运行时，请安装：pip install ai-edge-litert")
        if not os.path.exists(model_path):
            raise FileNotFoundError(f"模型缺失: {model_path}")

        self.interp = _Interpreter(model_path=model_path, num_threads=2)
        self.interp.allocate_tensors()
        self.in_idx = self.interp.get_input_details()[0]["index"]
        self.out_idx = self.interp.get_output_details()[0]["index"]

        self.labels = ["Open", "Close", "Pointer"]
        if os.path.exists(LABEL_PATH):
            with open(LABEL_PATH, encoding="utf-8-sig") as f:
                self.labels = [ln.strip() for ln in f if ln.strip()]

    def predict(self, landmark_list):
        """返回 (手势名, 置信度)"""
        feats = pre_process_landmark(landmark_list)
        arr = np.array([feats], dtype=np.float32)
        self.interp.set_tensor(self.in_idx, arr)
        self.interp.invoke()
        out = np.squeeze(self.interp.get_tensor(self.out_idx))

        # softmax 归一化成置信度（若模型已输出概率则直接取）
        e = np.exp(out - np.max(out))
        prob = e / e.sum()
        idx = int(np.argmax(prob))
        score = float(prob[idx])

        name = IDX_TO_GESTURE.get(idx, self.labels[idx].upper() if idx < len(self.labels) else f"UNK_{idx}")
        return name, score


# ── 单例 ──
_clf = None


def get_classifier():
    global _clf
    if _clf is None:
        _clf = GestureClassifier()
    return _clf


def classify_mlp(landmark_list):
    """兼容旧接口：返回手势名，失败回退 None"""
    try:
        name, _ = get_classifier().predict(landmark_list)
        return name
    except Exception as e:
        print(f"[GESTURE] MLP 推理失败: {e}")
        return None
