git_pull_10min
=================


Bizarre the 10min pull time quite repeatable !


::

    N[blyth@localhost opticks_bitbucket]$ ./pull.sh 
    Thu Aug  3 02:09:31 CST 2023
    remote: Enumerating objects: 22, done.
    remote: Counting objects: 100% (22/22), done.
    remote: Compressing objects: 100% (14/14), done.
    remote: Total 14 (delta 11), reused 0 (delta 0), pack-reused 0
    Unpacking objects: 100% (14/14), 2.47 KiB | 148.00 KiB/s, done.
    From https://bitbucket.org/simoncblyth/opticks
       982b14a23..36c722de0  master     -> origin/master
    Updating 982b14a23..36c722de0
    Fast-forward
     bin/rsync_put.sh                                                 | 10 ++++++++--
     notes/issues/rsync_put_repo_macOS_to_Linux_case_irregularity.rst | 66 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     pull.sh                                                          |  2 ++
     qudarap/tests/{qstateTest.cc => QStateTest.cc}                   |  0
     4 files changed, 76 insertions(+), 2 deletions(-)
     create mode 100644 notes/issues/rsync_put_repo_macOS_to_Linux_case_irregularity.rst
     rename qudarap/tests/{qstateTest.cc => QStateTest.cc} (100%)
    Thu Aug  3 02:19:23 CST 2023
    N[blyth@localhost opticks_bitbucket]$ 




    N[blyth@localhost opticks_bitbucket]$ ./pull.sh 
    origin	https://bitbucket.org/simoncblyth/opticks (fetch)
    origin	https://bitbucket.org/simoncblyth/opticks (push)
    Thu Aug  3 03:07:52 CST 2023
    remote: Enumerating objects: 38, done.
    remote: Counting objects: 100% (38/38), done.
    remote: Compressing objects: 100% (21/21), done.
    remote: Total 21 (delta 16), reused 0 (delta 0), pack-reused 0
    Unpacking objects: 100% (21/21), 3.98 KiB | 110.00 KiB/s, done.
    From https://bitbucket.org/simoncblyth/opticks
       36c722de0..aa7fa5f3f  master     -> origin/master
    Updating 36c722de0..aa7fa5f3f
    Fast-forward
     CSG/CSGFoundry.cc                    | 28 ++--------------------------
     CSG/CSGFoundry.h                     |  1 -
     qudarap/QEvent.cc                    | 15 +++++++++------
     qudarap/QEvent.hh                    |  6 +++++-
     sysrap/CMakeLists.txt                |  1 +
     sysrap/NP.hh                         | 27 +++++++++++++++++++++++++--
     sysrap/SEvt.cc                       | 91 ++++++++++++++++++++++++++-----------------------------------------------------------------
     sysrap/SEvt.hh                       |  4 ----
     sysrap/smeta.h                       | 54 ++++++++++++++++++++++++++++++++++++++++++++++++++++++
     sysrap/tests/SEvt_AddEnvMeta_Test.cc |  9 +++------
     sysrap/tests/smeta_test.cc           | 11 +++++++++++
     sysrap/tests/smeta_test.sh           | 29 +++++++++++++++++++++++++++++
     u4/U4Recorder.cc                     | 17 ++++++++++++-----
     u4/U4Recorder.hh                     |  5 ++++-
     14 files changed, 181 insertions(+), 117 deletions(-)
     create mode 100644 sysrap/smeta.h
     create mode 100644 sysrap/tests/smeta_test.cc
     create mode 100755 sysrap/tests/smeta_test.sh
    Thu Aug  3 03:17:44 CST 2023
    N[blyth@localhost opticks_bitbucket]$ 





    N[blyth@localhost opticks_bitbucket]$ ./pull.sh 
    origin	https://bitbucket.org/simoncblyth/opticks (fetch)
    origin	https://bitbucket.org/simoncblyth/opticks (push)
    Thu Aug  3 03:56:57 CST 2023
    remote: Enumerating objects: 28, done.
    remote: Counting objects: 100% (28/28), done.
    remote: Compressing objects: 100% (15/15), done.
    remote: Total 15 (delta 13), reused 0 (delta 0), pack-reused 0
    Unpacking objects: 100% (15/15), 2.49 KiB | 91.00 KiB/s, done.
    From https://bitbucket.org/simoncblyth/opticks
       aa7fa5f3f..4f9fd4e44  master     -> origin/master
    Updating aa7fa5f3f..4f9fd4e44
    Fast-forward
     notes/issues/git_pull_10min.rst | 69 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     sysrap/SEventConfig.cc          |  6 ++++++
     sysrap/SEvt.cc                  | 15 +++++++++++----
     sysrap/SEvt.hh                  |  1 +
     sysrap/tests/smeta_test.cc      |  1 +
     sysrap/tests/sstr_test.cc       | 22 +++++++++++++++++-----
     sysrap/tests/sstr_test.sh       |  1 +
     u4/U4Recorder.cc                |  3 +++
     8 files changed, 109 insertions(+), 9 deletions(-)
     create mode 100644 notes/issues/git_pull_10min.rst
    Thu Aug  3 04:06:49 CST 2023
    N[blyth@localhost opticks_bitbucket]$ 



Try github into junotop::

    date ; git clone https://github.com/simoncblyth/opticks.git opticks_github ; date


::

    N[blyth@localhost junotop]$ date ; git clone https://github.com/simoncblyth/opticks.git opticks_github ; date
    Thu Aug  3 04:54:56 CST 2023
    Cloning into 'opticks_github'...
    fatal: unable to access 'https://github.com/simoncblyth/opticks.git/': Failed connect to github.com:443; Connection timed out
    Thu Aug  3 04:57:04 CST 2023
    N[blyth@localhost junotop]$ 


bitbucket blockage seems fixed::

    N[blyth@localhost opticks_bitbucket]$ ./pull.sh 
    origin	https://bitbucket.org/simoncblyth/opticks (fetch)
    origin	https://bitbucket.org/simoncblyth/opticks (push)
    Mon Aug  7 16:34:00 CST 2023
    remote: Enumerating objects: 276, done.
    remote: Counting objects: 100% (276/276), done.
    remote: Compressing objects: 100% (204/204), done.
    remote: Total 204 (delta 172), reused 0 (delta 0), pack-reused 0
    Receiving objects: 100% (204/204), 46.06 KiB | 183.00 KiB/s, done.
    Resolving deltas: 100% (172/172), completed with 67 local objects.
    From https://bitbucket.org/simoncblyth/opticks
       4f9fd4e44..4b3c28f8e  master     -> origin/master
    Updating 4f9fd4e44..4b3c28f8e
    Fast-forward
     CSG/CSGFoundry.py                                                       |   8 +-
     CSGOptiX/CSGOptiX.cc                                                    |   9 +-
     CSGOptiX/cxs_min.py                                                     |   4 +-
     CSGOptiX/cxs_min.sh                                                     |   2 +-
     ana/fold.py                                                             |   7 ++
     examples/UseCustom4/go.sh                                               |  38 ++++---
     ...
     u4/tests/FewPMT_test.cc                                                 |  10 ++
     u4/tests/FewPMT_test.sh                                                 |   8 ++
     u4/tests/U4SimulateTest.sh                                              | 132 +++----------------------
     u4/tests/storch_FillGenstep.sh                                          | 151 ++++++++++++++++++++++++++++
     71 files changed, 2799 insertions(+), 432 deletions(-)
     create mode 100644 g4cx/tests/G4CXApp.h
     create mode 100644 g4cx/tests/G4CXAppTest.cc
     create mode 100644 g4cx/tests/G4CXAppTest.py
     create mode 100755 g4cx/tests/G4CXAppTest.sh
     create mode 100755 g4cx/tests/gx.sh
     delete mode 100755 g4cx/tests/ntds3.sh
     create mode 100644 notes/issues/qsim_propagate_at_surface_CustomART_NOT-A-SENSOR_error.rst
     create mode 100644 notes/issues/sphoton_iindex_identity_CPU_GPU_difference.rst
     create mode 100644 sysrap/srng.h
     rename u4/{tests => }/U4App.h (100%)
     create mode 100644 u4/U4Boundary.h
     create mode 100644 u4/tests/FewPMT_test.cc
     create mode 100755 u4/tests/FewPMT_test.sh
     create mode 100644 u4/tests/storch_FillGenstep.sh
    Mon Aug  7 16:34:06 CST 2023
    N[blyth@localhost opticks_bitbucket]$ 



