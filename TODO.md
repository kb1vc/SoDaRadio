0. Factor common UI code from shortsoda and qtgui into gui_common

1. Make the choice of a radio a mandatory command line param rather than
an optional switch. 

2. Move unix domain socket files from .local/share... to a temp
directory. Allow multiple instances of SoDaRadio to run (on
separate devices).

3. Improve handling of abend to make sure we've deleted UD sockets and shut down the server process. 

4. Add a keep-alive timer.

5. Make device-not-found failures more graceful. 

6. Document the process for connecting wsjtx or fldigi

7. Write a "new radio" howto.
