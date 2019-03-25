# Todo

* Bugs and crashes
  * Exceptions on exit.
  * Exit routine is sometimes forceful.
  * Ignore doesn't seem to work.
* Sync
  * Overwriting executable calls `exit(1)` instead of a graceful exit.
  * Acquire lock on file before sending it to prevent editing.
* Deploy disconnect fails to update main command handler state.
* SMTP is likely broken and needs debugging.
* What if 404 doesn't exist?
* 404 script undefined behavior.
* Error handling from CGI scripts.
* Reconnect after remote restart during sync.
