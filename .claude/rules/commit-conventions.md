# Rule: Commit Conventions

1. **Format:** every commit message, exactly:
   ```
   [Type] Short descriptive title (max 72 chars)

   Longer description wrapped at 72 characters explaining the rationale
   and technical details.

   Signed-off-by: Author Name <author.email@example.com>
   ```
2. **Allowed types:**
   - `[Feature]` — new capability
   - `[Fix]` — bug fix
   - `[Docs]` — documentation
   - `[Style]` — style, formatting, whitespace (no logic change)
   - `[Refactor]` — refactor, no public behavior change
   - `[Test]` — add/update tests
   - `[Chore]` — build system, Docker, CI, deps
   - `[Perf]` — performance
3. **Linear history:** feature branches rebased onto `main`, squash-merged.
4. **Push only, no `gh`:** finish by rebasing onto `origin/main`, then `git push origin <branch>`. Never use or install GitHub CLI (`gh`). User opens PRs on GitHub.
