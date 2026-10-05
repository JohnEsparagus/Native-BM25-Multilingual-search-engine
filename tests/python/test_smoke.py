import pytest
from multilingual_search import SearchEngine


def make_engine():
    engine = SearchEngine()
    engine.add_document("The whale was spotted near the ship.", "moby_dick")
    engine.add_document("Elizabeth Bennet met Mr Darcy.", "pride_and_prejudice")
    return engine


def test_search_returns_ranked_results():
    engine = make_engine()
    assert len(engine) == 2

    results = engine.search("white whale", 10)
    assert [r.title for r in results] == ["moby_dick"]
    assert results[0].score > 0
    assert results[0].doc_id == 0

    assert [r.title for r in engine.search("darcy")] == ["pride_and_prejudice"]

def test_chinese_search():
    engine = SearchEngine("chinese")
    engine.add_document("西门庆遇见了潘金莲", "jinpingmei")
    engine.add_document("今天天气很好，我们去公园散步", "other")

    results = engine.search("潘金莲", 10)
    assert [r.title for r in results] == ["jinpingmei"]
    assert results[0].score > 0

    assert engine.search("完全不存在的词语") == []
    
def test_stemming_and_no_match():
    engine = make_engine()
    assert [r.title for r in engine.search("whales")] == ["moby_dick"]
    assert engine.search("zzzzqqqq") == []


def test_top_k_limits_results():
    engine = make_engine()
    engine.add_document("A whale, another whale.", "extra")
    assert len(engine.search("whale", 1)) == 1
    assert len(engine.search("whale", 10)) == 2
    assert engine.search("whale", 0) == []


def test_empty_engine():
    assert SearchEngine().search("anything") == []


def test_save_and_load_roundtrip(tmp_path):
    path = str(tmp_path / "index.bin")
    make_engine().save(path)

    loaded = SearchEngine()
    loaded.load(path)
    assert len(loaded) == 2
    assert [r.title for r in loaded.search("whale")] == ["moby_dick"]


def test_errors_become_python_exceptions(tmp_path):
    with pytest.raises(ValueError):
        SearchEngine("klingon")
    with pytest.raises(ValueError):
        make_engine().search("whale", -1)
    with pytest.raises(RuntimeError):
        SearchEngine().load(str(tmp_path / "missing.bin"))