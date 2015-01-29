<?php
/**
 * @file
 * Enable/revert the components from the profile "*.info" file or file,
 * that stored in "sites/default/components.info".
 *
 * @example
 *   ; Features
 *   components[] = feature_name
 *   ; Modules
 *   components[] = modules_name
 *
 * @author Sergey Bondarenko <broken@propeople.com.ua>
 */

/**
 * Implements hook_drush_command().
 */
function update_project_components_statuses_drush_command() {
  $description = dt('Enable/revert the features from the profile "*.info" file or file, that stored in "sites/default" and named "components.info".');
  $commands = array();

  $commands['update-project-components-statuses'] = array(
    'description' => $description,
    'drupal dependencies' => array('features'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'aliases' => array('upcs'),
    'examples' => array(
      'drush frp' => $description,
    ),
  );

  return $commands;
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_update_project_components_statuses() {
  $profile_name = drupal_get_profile();
  $profile_path = drupal_get_path('profile', $profile_name);
  $components = array();
  $defined = array();

  foreach (array("$profile_path/$profile_name", 'sites/default/components') as $info_file) {
    $info_file = DRUPAL_ROOT . "/$info_file.info";

    if (file_exists($info_file)) {
      $info = drupal_parse_info_file($info_file);

      if (!empty($info['components']) && is_array($info['components'])) {
        $defined = array_merge($defined, $info['components']);
      }
    }
  }

  if ($defined) {
    module_load_include('inc', 'features', 'features.export');
    drush_include_engine('drupal', 'environment');

    $enabled_modules = module_list();
    $all_modules = drush_get_modules();

    foreach ($defined as $i => $component) {
      if (isset($all_modules[$component])) {
        /* @var \stdClass $feature */
        $feature = features_get_features($component);

        if ($feature && features_get_storage($feature->name) == FEATURES_OVERRIDDEN) {
          $components['revert'][] = $component;
        }

        if (!isset($enabled_modules[$component]) && isset($all_modules[$component])) {
          $info = drush_check_module_dependencies(array($component), $all_modules);

          foreach ($info[0]['dependencies'] as $dependency) {
            if (!isset($enabled_modules[$dependency])) {
              $components['enable'][] = $dependency;
            }
          }

          $components['enable'][] = $component;
        }
      }
      else {
        // Component could not be found.
        $components['missing'][] = $component;
      }
    }
  }

  if ($components) {
    foreach ($components as $group => $extensions) {
      $components['strings'][$group] = array("!$group" => implode(', ', $extensions));
    }
  }

  // Enable disabled components.
  if (empty($components['enable'])) {
    drush_log(dt('Nothing to enabling.'), 'ok');
  }
  else {
    drush_set_context('PM_ENABLE_MODULES', $components['enable']);
    drush_pm_enable();
  }

  // Revert overridden features.
  if (empty($components['revert'])) {
    drush_log(dt('Nothing to revert.'), 'ok');
  }
  else {
    if (drush_confirm(dt('Would you like to revert the next features: !revert', $components['strings']['revert']))) {
      call_user_func_array('drush_features_revert', $components['revert']);
    }
  }

  // Inform about missing components.
  if (!empty($components['missing'])) {
    drush_log(dt('The following extensions are missing in your installation and could not be enabled: !missing', $components['strings']['missing']), 'warning');
  }
}
