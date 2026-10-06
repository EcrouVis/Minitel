# Emulateur M12



Emulateur hardware d'un Minitel 12 Philips



## Compilation



### Prérequis



* Un compilateur gcc ou llvm (gcc de préférence) (pour windows mingw-w64)
* CMake
* NSIS installé et configuré pour empaqueter un installateur
* appimagetool installé et configuré pour empaqueter une appimage
* MbedTLS compilé et installé pour windows (OpenSSL et LibreSSL peuvent normalement fonctionner mais cela demande plus d'efforts)
* MbedTLS, OpenSSL ou LibreSSL installé pour les autres distributions
* Les dépendances de GLFW (pour toutes les distributions excepté windows / voir: https://www.glfw.org/docs/latest/compile.html)



### Scripts



Pour simplifier la compilation et l'empaquetage, plusieurs scripts sont fournis:

* build\_appimage.sh pour créer une appimage
* build\_installer.bat pour créer un instalateur
* build\_portable.bat pour créer un exécutable portable
* build\_test.bat pour itérer facilement et tester le code



## Dépendances



* Dear ImGui pour l'interface graphique des paramètres (inclus et compilé automatiquement)

  * GLFW pour simplifier l'affichage et le clavier (téléchargé et compilé automatiquement)

    * glad et les définitions KHR pour charger OpenGL (inclus et compilé automatiquement)
    * d'autres dépendances en fonction de la distribution (doivent être installées préalablement)
* cJSON pour l'enregistrement des paramètres (inclus et compilé automatiquement)
* IXWebsocket pour se connecter avec des websockets (téléchargé et compilé automatiquement)

  * MbedTLS, OpenSSL ou LibreSSL pour se connecter avec des websockets sécurisés (doit être installé préalablement)
* miniaudio pour simplifier la gestion des entrées/sorties audio (inclus et compilé automatiquement)
* d'autres dépendances liées au compilateur et à la distribution (inclus avec le compilateur)



## Structure du code



Arborescence du code:

* src: code et fichiers sources

  * circuit: relatif au code simulant les parties du minitel
  * desktop: relatif à des spécificités de la distribution
  * autres dossiers: dépendances
  * main.cpp: point d'entrée
  * main\_circuit.cpp: point d'entrée pour le thread de la simulation / défini les connexions entre les composants simulés et lance la simulation
  * main\_video.cpp: point d'entrée pour le thread de l'interface graphique / gère le thread audio, et s'occupe des entrées sorties et de l'interface
  * autres fichiers: non classé
* include: définitions et code

  * circuit: relatif au code simulant les parties du minitel
  * io: relatif à du code pour l'adaptation des entrées/sorties
  * desktop: relatif à des spécificités de la distribution
  * autres dossiers: dépendances
  * autres fichiers: non classé
* install: dossier contenant l'exécutable créé par build\_test.bat et ses fichiers
* out: dossier contenant les paquets créés par les autres scripts

