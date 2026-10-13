# How we work in this repo

## First time

1. Accept the collaborator invite (check your email or github.com/notifications).
2. `git clone https://github.com/Chingy0404/cg2271-as4.git`
3. Create `esp32/as4_esp32/secrets.h` from `secrets.example.h`. It is ignored by Git — **never commit passwords or API keys.** If one is pushed by accident, rotate the key; deleting the commit isn't enough.

No command line? GitHub Desktop (desktop.github.com) does everything below with buttons.

## Every time you work on something

```bash
git checkout main
git pull                                  # get everyone else's work first
git checkout -b feature/ultrasonic-sensor # one branch per part
# ...edit, build, test on the board...
git add <files you changed>
git commit -m "Add ultrasonic polling task"
git push -u origin feature/ultrasonic-sensor
```

Then open a **Pull Request** on GitHub into `main`, and ask one teammate to look at it before merging. Merging small PRs often is much easier than one big merge the night before the deadline.

Branch names: `feature/<part>`, `fix/<what>`, `docs/<what>`.

## Rules that save us pain

- **Only commit code that builds.** Test on the board when you can.
- **Protocol changes:** edit `common/protocol.h` only, run `./scripts/sync_protocol.sh`, update `docs/protocol.md`, and tell the person working on the other board.
- **Pins:** claim them in `docs/hardware.md` before wiring.
- **Shared files** (`FreeRTOSConfig.h`, `pin_mux.c`, `main.c` task creation): keep changes small and mention them in the group chat.
- Put your driver in its own `.c/.h` file — fewer merge conflicts.
- Don't commit `Debug/`, `.metadata/` or build output (already in `.gitignore`).

## Merge conflict?

`git pull` says CONFLICT → open the file, look for `<<<<<<<`, `=======`, `>>>>>>>`, keep the right parts, delete the markers, then `git add <file>` and `git commit`. Ask the other person if you're not sure whose version is right.
