#include "cameo/src/browser/shell_registry.h"

#include "cameo/src/common/cameo_notification_types.h"
#include "cameo/src/browser/shell.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

using content::RenderViewHost;

namespace cameo {

// A application-wide shell registry.
static ShellRegistry* g_shell_registry = NULL;

ShellRegistry::ShellRegistry() {
  DCHECK(g_shell_registry == NULL);
  g_shell_registry = this;
}

ShellRegistry::~ShellRegistry() {
  DCHECK(g_shell_registry);
  DCHECK(shell_vector_.empty()) << "Shell instances are not erased completely";
  g_shell_registry = NULL;
}

// static
ShellRegistry* ShellRegistry::Get() {
  return g_shell_registry;
}

void ShellRegistry::AddObserver(ShellRegistryObserver* obs) {
  observer_list_.AddObserver(obs);
}

void ShellRegistry::RemoveObserver(ShellRegistryObserver* obs) {
  observer_list_.RemoveObserver(obs);
}

void ShellRegistry::AddShell(Shell* shell) {
  shell_vector_.push_back(shell);

  content::NotificationService::current()->Notify(
      cameo::NOTIFICATION_SHELL_OPENED,
      content::Source<Shell>(shell),
      content::NotificationService::NoDetails());

  FOR_EACH_OBSERVER(ShellRegistryObserver, observer_list_, OnShellAdded(shell));
}

void ShellRegistry::RemoveShell(Shell* shell) {
  ShellVector::iterator it =
      std::find(shell_vector_.begin(), shell_vector_.end(), shell);
  if (it != shell_vector_.end())
    shell_vector_.erase(it);

  content::NotificationService::current()->Notify(
      cameo::NOTIFICATION_SHELL_CLOSED,
      content::Source<Shell>(shell),
      content::NotificationService::NoDetails());

  FOR_EACH_OBSERVER(ShellRegistryObserver, observer_list_,
                    OnShellRemoved(shell));
}

Shell* ShellRegistry::GetShellFromRenderViewHost(
    RenderViewHost* render_view_host) {
  for (ShellVector::iterator it = shell_vector_.begin();
       it != shell_vector_.end(); ++it) {
    if ((*it)->web_contents()->GetRenderViewHost() == render_view_host)
      return (*it);
  }
  return NULL;
}

void ShellRegistry::CloseAll() {
  ShellVector cached_shells;

  ShellVector::iterator it = shell_vector_.begin();
  for(; it != shell_vector_.end(); ++it) {
    cached_shells.push_back(*it);
  }

  for (it = cached_shells.begin(); it != cached_shells.end(); ++it)
    (*it)->Close();

  // If a Shell is closed, it will be deleted by itself and also be removed
  // from ShellRegistry. The shell vector should be empty after all Shells are
  // closed.
  DCHECK(shell_vector_.size() == 0);
}

}  // namespace cameo
