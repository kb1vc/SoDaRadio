0. [done] Factor common UI code from shortsoda and qtgui into gui_common

1. [done] Make the choice of a radio a mandatory command line param rather than
an optional switch. 

2. [done] Move unix domain socket files from .local/share... to a temp
directory. Allow multiple instances of SoDaRadio to run (on
separate devices).

3. [done] Improve handling of abend to make sure we've deleted UD sockets and shut down the server process. 

4. Add documentation on config files. 

5. Add a keep-alive timer.

6. Make device-not-found failures more graceful. 

7. Document the process for connecting wsjtx or fldigi

8. Write a "new radio" howto.
