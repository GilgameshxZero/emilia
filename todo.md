# Todo

* Bugs and crashes
  * Exceptions on exit.
  * Exit routine is sometimes forceful.
* Sync
  * Overwriting executable calls `exit(1)` instead of a graceful exit.
  * Acquire lock on file before sending it to prevent editing.
* Deploy disconnect fails to update main command handler state.
* SMTP is likely broken and needs debugging.
