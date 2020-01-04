# Percent literals

If only a single latin letter follows the percentage sign, and then one of the brackets (`([{<`), then it's a magic literal, which expansion depends on the letter:

  * `%q` — a **q**uoted `Twine`, e.g. `%q["foo"] == '"foo"'`
  * `%Q` — a quoted `String` with interpolation enabled
  * `%w` — **w**ords, a slice of space-separated twines,
  e.g. `%w(foo bar) == {'foo', 'bar'}`
  * `%W` — string words with interpolation enabled
  * `%g` — a slice of `Glyph`s, e.g. ``%g<a✓`> == {`a`, `✓`, ```}``
  * Just `%` is the same as `%q` (i.e. no interpolation)

In case if there are two or more letters, it's a heredoc then. Depending on the **first** letter casing, they act exactly as either `%q` or `%Q`. The main advantage of heredocs is that text editors typically provide syntax highlighting for them.

Multiline percent literals are aligned at the least indentation. You may want to use escaped spaces (`\ ` or `\s`) in edge cases. Note that the canonical formatter always makes the least indentation to indent exactly two spaces from the beginning of the line, and the end bracket is placed on a new line.

```onyx
# Already well formatted
%sql(
  SELECT #{foo}
    FROM bar
) == 'SELECT #{foo}\n  FROM bar'

%Sql<
  SELECT #{foo}
FROM bar> == "  SELECT #{foo}\nFROM bar"
# Would be formatted to:
%Sql<
    SELECT #{foo}
  FROM bar
> == ".."

%SQL[SELECT
  FROM bar\s
    WHERE baz] == "SELECT\nFROM bar \n  WHERE baz"
# Would be formatted to:
%SQL[
  SELECT
  FROM bar\s
    WHERE baz
] == ".."
```
