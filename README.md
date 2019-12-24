# NextGIS standard Qt library

NextGIS standard Qt library implements standard API for console or GUI applications
and my.nextgis.com service.

The library has the following functionality:

* Basic CoreApplication for command line parsing, translation loading and Settings
* Basic GUIApplication for create main windows, panes, styles (dark and light),
  UI specific controls, etc.
* oAuth2 authorization to my.nextgis.com using login/password or code flow.
  Store refresh and connect tokens.
* Exit from account
* Fetch user details from my.nextgis.com including current plan and it expire period,
  Web GIS, etc.
* Notify application about some events (refresh oAuth token failed, server side
  messages, finished of processing tasks and etc.).
* [Future] Account management (change plan, change details, Web GIS activities, etc.)
* Check for updates using nextgisupdater application

# Supported software

The following software use NextGIS authorization capabilities of NextGIS standard
Qt library:

* NextGIS QGIS
* NextGIS QGIS plugings:
  * NextGIS Connect [planned]
  * NextGIS Data [planned]
  * QuickMapServices [planned]
  * DTClassifier
* NextGIS Formbuilder
* NextGIS Manager [planned]
* NextGIS Manuscript

# Bindings

Library has C++ API and Python bindings using sip.

# References

Best practices for oAuth in desktop applications:

* https://auth0.com/blog/oauth-2-best-practices-for-native-apps/
* https://developers.google.com/identity/protocols/OAuth2InstalledApp
* http://wiki.oauth.net/w/page/27249271/OAuth%202%20for%20Native%20Apps
* https://tools.ietf.org/html/draft-ietf-oauth-native-apps-03

Antipattern:

* https://developers.arcgis.com/qt/10-2/cpp/guide/use-oauth-2-0-authentication.htm

# License

All scripts are licensed under GNU GPL v2, or (at your option) any later version.
See COPYING file.

![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg?maxAge=2592000)

# Commercial support

Need to fix a bug or add a feature to NextGIS installer? We provide custom
development and support for this software.
[Contact us](https://nextgis.ru/en/contact/) to discuss options!

[![https://nextgis.com](https://nextgis.ru/img/nextgis.png)](https://nextgis.com)

# Borsch

[NextGIS Borsch](https://github.com/nextgis-borsch/borsch) helps to resolve
dependencies of building C/C++ libraries and applications. NextGIS Borsch is based on [CMake](https://cmake.org/).

[![Borsch compatible](https://img.shields.io/badge/Borsch-compatible-orange.svg?style=flat)](https://github.com/nextgis-borsch/borsch)
