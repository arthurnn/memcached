dnl  Copyright (C) 2009 Sun Microsystems
dnl This file is free software; Sun Microsystems
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([PANDORA_BUILDING_FROM_VC],[

  ac_cv_building_from_vc=no

  AS_IF([test -d "${srcdir}/.bzr"],[
    ac_cv_building_from_bzr=yes
    ac_cv_building_from_vc=yes
    ],[
    ac_cv_building_from_bzr=no
  ])

  AS_IF([test -d "${srcdir}/.svn"],[
    ac_cv_building_from_svn=yes
    ac_cv_building_from_vc=yes
    ],[
    ac_cv_building_from_svn=no
  ])

  AS_IF([test -d "${srcdir}/.hg"],[
    ac_cv_building_from_hg=yes
    ac_cv_building_from_vc=yes
    ],[
    ac_cv_building_from_hg=no
  ])

])
  
