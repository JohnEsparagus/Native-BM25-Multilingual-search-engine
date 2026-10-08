from pathlib import Path


def load_qrels(path):
    """Parse 'query => file1 | file2'. Returns [(query, {filenames})].
    Same rules as load_eval_queries() in src/evaluation.cpp:
    skip blank lines, '#' lines, and lines without '=>'."""
    qrels = []
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=>" not in line:
            continue
        query, rel = line.split("=>", 1)
        query = query.strip()
        relevant = {name.strip() for name in rel.split("|") if name.strip()}
        if query and relevant:
            qrels.append((query, relevant))
    return qrels