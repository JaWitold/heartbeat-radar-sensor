# Rule: Writing Style (Docs, Comments, Rules, UI Text)

Scope: `README.md`, `CLAUDE.md`, `docs/`, `.claude/rules/`, `.claude/skills/`, code comments, docstrings, script help/banners, log messages, UI labels.

1. **Technical only.** What it is, what it does, how to use it, limits. Nothing else.
2. **No marketing/theme words.** Banned: game/franchise refs (Call of Duty, MW2, Rainbow Six, R6, ...), "tactical", "military", "mission", hype adjectives ("powerful", "blazing", "seamless", "robust", "comprehensive", "cutting-edge", "immersive", "elegant", "real-time" without rate). No exclamation marks.
3. **Concrete over vague.** Numbers, units, names, paths: `256000 baud`, `60 FPS`, `src/core/`, `LD2450Adapter`. Not "high-speed", "fast", "the parser".
4. **Short — caveman full.** `README.md`, `CLAUDE.md`, `docs/`, `.claude/rules/`, `.claude/skills/`: drop articles, filler, hedging; fragments OK; short synonyms; one fact per line. Code blocks, commands, paths, tables unchanged. Code comments: terse but grammatical enough to read. UI labels + log messages: not caveman, just short. Delete any sentence with no fact.
5. **No decoration.** No emoji in headings, comments, logs, UI. No badges not reflecting real state. No sign-off flourishes.
6. **Comments = why, not what.** Don't restate code. Document units, wire formats, invariants, non-obvious constraints.
7. **Accurate.** Documented paths, flags, commands, versions, test frameworks match repo. Code changes → update docs in same commit. Mark stubs + planned features.
8. **Rules imperative.** "Use X", "Never Y". State each rule once; link from elsewhere, don't repeat.
