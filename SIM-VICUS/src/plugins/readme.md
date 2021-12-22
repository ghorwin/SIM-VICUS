# SIM-VICUS Plugin-Interfaces

This directory contains abstract interface class declarations that can
be inherited by plugins and implemented, to extend the SIM-VICUS user interface.

## Implementing Plugins

Depending on the plugin, the exact same versions of 
the VICUS library and SIM-VICUS classes shall be used. For example,
if you develop an import plugin and populate the VICUS-project data structure,
you *must* compile against the same VICUS library.

Similarly, when you access singletons from SIM-VICUS, like `SVProjectHandler` or
`SVSettings`, you must always compile against the same headers.

In order to ensure this, we introduce plugin-interface-versioning. This
interface version (which is different from SIM-VICUS/VICUS versioning) 
is increased always, when either `VICUS`-library, 
`SVProjectHandler` or `SVSettings` change their data structures. The 
plugin-loader will reject not matching plugin versions.

## Available Interfaces

- `ImportPluginInterface.h` - declares the plugin-interface for project 
  importers, i.e. plugins that populate the VICUS project structure
  
- `DatabasePluginInterface.h` - declares the interface for a plugin that
  provides database content, similar to the built-in database. When the 
  plugin is loaded, the provided data is simply added as built-in data
  to the database.
  
## Common plugin functionality

- All plugins will may provide configuration functionality. For that purpose
  the common plugin base class provides member functions to signal
  the availability of a configuration/settings function (if provided,
  a menu entry is added for configuration), and the respective configuration
  function.



