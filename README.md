JMVC - Joint Multiview Video Coding
===================================

This software is the reference software for the Multiview Video Coding (MVC) project of the Joint video Team (JVT) of the ISO/IEC Moving Pictures Experts Group (MPEG) and the ITU-T Video Coding Experts Group (VCEG). This software is **not** owned by me. The first commit in this git repository represents the official software at version 8.5 (CVS tag: JMVC\_8\_5), which can be found by accessing their CVS repository:

```
cvs -d :pserver:jvtuser:jvt.Amd.2@garcon.ient.rwth-aachen.de:/cvs/jvt login
cvs -d :pserver:jvtuser@garcon.ient.rwth-aachen.de:/cvs/jvt checkout jmvc
```

Only subsequent commits in this git repository are authored by me.

My changes include
- Allowing it to compile on my Linux system (changes have not been tested on Windows)
- Exchanging the .doc manual for a PDF
- Changing the output format for better readability and automated parsing
- Adding the ability to write motion vectors in a human-readable, script-parsable format
