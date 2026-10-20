# Madeira reading progress bridge v1 - managed by Madeira.
# Optional, read-only counters. No save/persistent writes, script scan, timers,
# threads, screens, input hooks, or dialogue content exported. Python 2/3.
init 999 python hide:
    def madeira_install_reading_progress_v1():
        try:
            import json
            import os
            import time
            import renpy

            folder = renpy.config.gamedir
            control = os.path.join(folder, ".madeira-reading-session-v1.json")
            output = os.path.join(folder, ".madeira-reading-progress-v1.json")
            with open(control, "rb") as stream:
                initial = json.loads(stream.read(1024))
            token = initial.get("session")
            if initial.get("schema") != 1 or not isinstance(token, type(u"")) or len(token) != 36:
                return
            if not callable(getattr(renpy, "count_dialogue_blocks", None)) or not callable(getattr(renpy, "count_seen_dialogue_blocks", None)):
                return

            # The instance lives only in config's callback list, outside game
            # stores, save files and rollback. Captured modules stay local.
            class Reporter(object):
                def __init__(self):
                    self.last_check = None
                    self.last_counts = None
                    self.total = None
                    self.failed = False

                def sample(self):
                    if self.failed:
                        return
                    now = time.time()
                    if self.last_check is not None and 0 <= now - self.last_check < 2.0:
                        return
                    self.last_check = now
                    try:
                        with open(control, "rb") as stream:
                            settings = json.loads(stream.read(1024))
                        if settings.get("session") != token:
                            self.failed = True
                            return
                        if not settings.get("enabled"):
                            return
                        if self.total is None:
                            self.total = int(renpy.count_dialogue_blocks())
                        seen = int(renpy.count_seen_dialogue_blocks())
                        counts = (seen, self.total)
                        if counts == self.last_counts:
                            return
                        payload = json.dumps(dict(schema=1, session=token, seen=seen, total=self.total), separators=(",", ":")).encode("ascii")
                        # A bounded write only when counts change. The native
                        # reader ignores incomplete JSON and retries later;
                        # no fsync or forced persistent-save stalls the game.
                        with open(output, "wb") as stream:
                            stream.write(payload)
                        self.last_counts = counts
                    except Exception:
                        # Optional telemetry must never stop a story, even if
                        # the folder is read-only or the game overrides an API.
                        self.failed = True

            renpy.config.interact_callbacks.append(Reporter().sample)
        except Exception:
            pass

    madeira_install_reading_progress_v1()
