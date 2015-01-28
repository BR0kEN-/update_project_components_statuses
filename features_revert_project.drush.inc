<?php
/**
 * @file
 * Enable/revert the features from the profile "*.info" file or file, that
 * stored in "sites/default/components.info". An array of modules/features
 * should be named "components".
 *
 * @example
 *   components[] = feature_name
 *
 * @author Sergey Bondarenko <broken@propeople.com.ua>
 */

/**
 * Implements hook_drush_command().
 */
function features_revert_project_drush_command() {
  $commands = array();

  $commands['features-revert-project'] = array(
    'description' => dt('Revert/enable the project features/modules that collected in special file.'),
    'drupal dependencies' => array('features'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'aliases' => array('frp'),
    'examples' => array(
      'drush frp' => dt('Enable/revert the features from the profile "*.info" file or file, that stored in "sites/default" and named "components.info".'),
    ),
  );

  return $commands;
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_features_revert_project() {
  $profile_name = drupal_get_profile();
  $profile_path = drupal_get_path('profile', $profile_name);
  $components = array();

  foreach (array("$profile_path/$profile_name", 'sites/default/components') as $info_file) {
    $info_file = DRUPAL_ROOT . "/$info_file.info";

    if (file_exists($info_file)) {
      $info = drupal_parse_info_file($info_file);

      if (!empty($info['components']) && is_array($info['components'])) {
        $components = array_merge($components, $info['components']);
      }
    }
  }

  if ($components) {
    module_load_include('inc', 'features', 'features.export');
    drush_include_engine('drupal', 'environment');

    $modules = module_list();

    foreach ($components as $i => $feature_name) {
      /* @var \stdClass $feature */
      $feature = features_get_features($feature_name);

      // If feature does not exist or overridden, then no need to revert it.
      if (!$feature || !features_get_storage($feature->name) == FEATURES_OVERRIDDEN) {
        unset($components[$i]);
      }

      // If features was not enabled, then we propose to do that.
      if (!isset($modules[$feature_name])) {
        if (drush_confirm(dt('Would you like enable the "!module"?', array('!module' => $feature_name)))) {
          if (module_enable(array($feature_name))) {
            drush_log(dt('The "!module" has been enabled.', array('!module' => $feature_name)));
          }
          else {
            drush_log(dt('"!module" was not found.', array('!module' => $feature_name)), 'error');
          }
        }
      }
    }
  }

  // Now, in the "$components" array there were only features that can be
  // reverted.
  if ($components) {
    if (drush_confirm(dt('Would you like to revert the next features: "!features"', array('!features' => implode('", "', $components))))) {
      call_user_func_array('drush_features_revert', $components);
    }
  }
  else {
    drush_log(dt('Nothing to revert or enable.'), 'success');
  }

  return NULL;
}
