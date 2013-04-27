#ifndef CAMEO_SRC_BROWSER_SHELL_REGISTRY_H_
#define CAMEO_SRC_BROWSER_SHELL_REGISTRY_H_

#include "base/observer_list.h"

namespace content {
class RenderViewHost;
};

namespace cameo {

class Shell;

// The observer for shell registry changes, e.g. a new Shell instance is added,
// or a Shell instance is removed.
class ShellRegistryObserver {
 public:
  // Called when a new Shell instance is added.
  virtual void OnShellAdded(Shell* shell) = 0;

  // Called when a Shell instance is removed.
  virtual void OnShellRemoved(Shell* shell) = 0;

 protected:
  virtual ~ShellRegistryObserver() {}
};

typedef std::vector<Shell*> ShellVector;

// ShellRegistry maintains for Shell instances created for app running.
class ShellRegistry {
 public:
  // Get the singleton instance of ShellRegistry.
  static ShellRegistry* Get();

  ShellRegistry();
  ~ShellRegistry();

  // Add/remove shell.
  void AddShell(Shell* shell);
  void RemoveShell(Shell* shell);

  // Find a shell from a RenderViewHost
  Shell* GetShellFromRenderViewHost(content::RenderViewHost* render_view_host);
  const ShellVector& shells() { return shell_vector_; }

  // Close all running Shell instances.
  void CloseAll();

  // Add/remove observer.
  void AddObserver(ShellRegistryObserver* obs);
  void RemoveObserver(ShellRegistryObserver* obs);

 private:
  ShellVector shell_vector_;

  ObserverList<ShellRegistryObserver> observer_list_;
};

}  // namespace cameo

#endif  // CAMEO_SRC_BROWSER_SHELL_REGISTRY_H_
