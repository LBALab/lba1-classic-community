del tempo.obj
del tempo.obs
copy /b version.obj perso.obj object.obj global.obj flipbox.obj diskfunc.obj fiche.obj extra.obj incrust.obj grille.obj grille_a.obj func.obj cpymask.obj Message.obj ambiance.obj Balance.obj gamemenu.obj fire.obj geretrak.obj gerelife.obj HoloMap.obj playfla.obj adfli_a.obj mcga.obj tempo.obs
ren tempo.obs tempo.obj
wcl386 /l=dos4g /x /zq /4s /k7000 /fe=LBA0.exe tempo.obj
