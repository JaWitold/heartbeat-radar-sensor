# Rule: Commit Conventions

1. **Format:** Every commit message must follow this exact format:
   ```
   [Type] Short descriptive title (max 72 chars)

   Longer description wrapped at 72 characters explaining the rationale
   and technical details.

   Signed-off-by: Author Name <author.email@example.com>
   ```
2. **Allowed Types:**
   - `[Feature]` — New capability or functionality
   - `[Fix]` — Bug fix
   - `[Docs]` — Documentation changes
   - `[Style]` — Code style, formatting, whitespace (no logic changes)
   - `[Refactor]` — Code refactoring without changing public behavior
   - `[Test]` — Adding or updating test suites
   - `[Chore]` — Build system, Docker, CI, or dependency updates
   - `[Perf]` — Performance optimization
3. **Linear History:** All feature branches must be rebased onto `main` and squash-merged.
4. **Push only, no `gh`:** Finish by rebasing onto `origin/main` and `git push origin <branch>`. Never use the GitHub CLI (`gh`) and never install it; the user opens pull requests on GitHub.
