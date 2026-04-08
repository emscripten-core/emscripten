# Emscripten Design Documents

This directory contains design documents for emscripten features and major
changes/refactors.

We are experimenting with keeping these documents here under source control with
the hope that this will increase understandability of the codebase.  This has
some advantages over doing all our planning in Google Docs or GitHub issues.
For example, it allows us to track the history of designs and it allows them to
be searchable using standard tools like `git grep`.

## Document Format

Each document in this directory should be a markdown file.  At the top of each
document should be a `Status` which can be either `Draft`, `Accepted`,
`Completed`.

When a document is marked as `Completed` it should also be updated such that
it is clear the work has been done, and is now in the past.  For example,
phrases such as "The current behavior" should be replaced with "The previous
behavior".
