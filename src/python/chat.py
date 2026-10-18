import anthropic

MODEL = "claude-opus-5-5"

SYSTEM = """You answer questions about a small library of English, Chinese and Arabic books.
Use only the numbered passages you are given, and cite them like [2].
If the passages don't answer the question, say so instead of guessing.
Reply in the language of the question, in plain text without markdown."""

_client = None


def build_prompt(question, hits):
    passages = "\n\n".join(f"[{n}] ({h['book']})\n{h['text']}" for n, h in enumerate(hits, 1))
    return f"<passages>\n{passages}\n</passages>\n\nQuestion: {question}"


def answer(question, hits):
    """Streams Claude's answer as text chunks, grounded in the retrieved passages."""
    global _client
    if _client is None:
        _client = anthropic.Anthropic()  # reads ANTHROPIC_API_KEY

    try:
        with _client.beta.messages.stream(
            model=MODEL,
            max_tokens=2000,
            system=SYSTEM,
            output_config={"effort": "low"},
            betas=["server-side-fallback-2026-07-01"],
            fallbacks="default",
            messages=[{"role": "user", "content": build_prompt(question, hits)}],
        ) as stream:
            yield from stream.text_stream
            if stream.get_final_message().stop_reason == "refusal":
                yield "\n\n(The model declined to answer this question.)"
    except anthropic.APIError as e:
        # headers are already sent, so report the failure inside the stream
        yield f"\n\n(The answer could not be finished: {e.__class__.__name__})"
