#!/usr/bin/env python3
import torch

def your_cosine_similarity(v1, v2):
    dot_product = torch.dot(v1,v2)
    norm_v1 = torch.norm(v1)
    norm_v2 = torch.norm(v2)
    cos = dot_product / (norm_v2 * norm_v1)
    return cos.item()
    

def test_harness():
    device = torch.device("cpu")
    print("Running Cosine Similarity Verification Harness...\n")

    # ----------------------------------------------------
    # TEST CASE 1: Identical vectors (Expected Score: 1.0)
    # ----------------------------------------------------
    a1 = torch.tensor([1.0, 2.0, 3.0], device=device)
    b1 = torch.tensor([1.0, 2.0, 3.0], device=device)
    
    sim1 = your_cosine_similarity(a1, b1)
    print(f"Test Case 1 (Identical)    -> Expected: 1.0, Got: {sim1:.6f}")

    # ----------------------------------------------------
    # TEST CASE 2: Orthogonal/Perpendicular vectors (Expected Score: 0.0)
    # ----------------------------------------------------
    a2 = torch.tensor([1.0, 0.0], device=device)
    b2 = torch.tensor([0.0, 1.0], device=device)
    sim2 = your_cosine_similarity(a2, b2)
    print(f"Test Case 2 (Orthogonal)   -> Expected: 0.0, Got: {sim2:.6f}")


if __name__ == "__main__":
    test_harness()
