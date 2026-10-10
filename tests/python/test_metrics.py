import math, sys
from pathlib import Path
import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "src" / "python"))
from metrics import (recall_at_k, reciprocal_rank, precision_at_k,
                     average_precision, ndcg_at_k)

def test_recall_basic():
    assert recall_at_k(["a", "b", "c"], {"a", "c", "z"}, 2) == pytest.approx(1/3)

def test_recall_hit_ranked_11th_with_k10():
    ranked = [f"x{i}" for i in range(10)] + ["a"]
    assert recall_at_k(ranked, {"a"}, 10) == 0.0

def test_recall_empty_relevant_raises():
    with pytest.raises(ValueError):
        recall_at_k(["a"], set(), 5)

def test_rr_first_hit_rank_3():
    assert reciprocal_rank(["x", "y", "a"], {"a"}, 10) == pytest.approx(1/3)

def test_rr_no_hit():
    assert reciprocal_rank(["x", "y"], {"a"}, 10) == 0.0

def test_rr_respects_k():
    assert reciprocal_rank(["x", "y", "a"], {"a"}, 2) == 0.0

def test_precision():
    assert precision_at_k(["a", "b", "c", "d"], {"a", "c"}, 4) == 0.5

def test_precision_k_larger_than_ranked_divides_by_k():
    # decision: missing slots count as misses
    assert precision_at_k(["a"], {"a"}, 5) == pytest.approx(0.2)

def test_average_precision():
    # hits at rank 1 and 3: (1/1 + 2/3) / |relevant|=2
    assert average_precision(["a", "b", "c"], {"a", "c"}, 3) == pytest.approx(5/6)

def test_ndcg_perfect_is_one():
    assert ndcg_at_k(["a", "b", "x"], {"a", "b"}, 3) == pytest.approx(1.0)

def test_ndcg_hand_computed():
    # DCG  = 1/log2(3) + 1/log2(4) = 1.1309
    # IDCG = 1/log2(2) + 1/log2(3) = 1.6309
    assert ndcg_at_k(["x", "a", "b"], {"a", "b"}, 3) == pytest.approx(0.6934, abs=1e-4)

def test_ndcg_no_hits():
    assert ndcg_at_k(["x", "y"], {"a"}, 2) == 0.0