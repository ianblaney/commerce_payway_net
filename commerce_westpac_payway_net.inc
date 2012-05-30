<?php

/**
 * Get PayWay Net token.
 *   Cribbed from http://github.com/GiantRobot/uc_payway_net/
 */
function _commerce_westpac_payway_net_get_token($parameters) {
  $cert_file = $parameters['cert_path'];
  $payway_base_url = $parameters['payway_url'];
  // Find the port setting, if any.
  // @TODO: parse_url().
  $payway_url = $payway_base_url;
  $port = 443;
  $port_pos = strpos($payway_base_url, ":", 6);
  $url_end_pos = strpos($payway_base_url, "/", 8);
  if ($port_pos !== FALSE && $port_pos < $url_end_pos) {
    $port = (int)substr($payway_base_url, ((int) $port_pos) + 1, ((int) $url_end_pos));
    $payway_url = substr($payway_base_url, 0, ((int) $port_pos)) . substr($payway_base_url, ((int) $url_end_pos), strlen($payway_base_url));
  }

  $ch = curl_init($payway_url . "RequestToken");
  if ($port != 443) {
    curl_setopt($ch, CURLOPT_PORT, $port);
  }
  curl_setopt($ch, CURLOPT_FAILONERROR, TRUE);
  curl_setopt($ch, CURLOPT_FORBID_REUSE, TRUE);
  curl_setopt($ch, CURLOPT_FRESH_CONNECT, TRUE);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  // Set timeout options.
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 30);
  curl_setopt($ch, CURLOPT_TIMEOUT, 30);
  // Set references to certificate files.
  curl_setopt($ch, CURLOPT_CAINFO, $cert_file);
  // Check the existence of a common name in the SSL peer's certificate
  // and also verify that it matches the hostname provided.
  curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 1);
  // Verify the certificate of the SSL peer.
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, TRUE);
  // Build the parameters string to pass to PayWay.
  $parameters_string = '';
  $init = TRUE;
  foreach ($parameters as $name => $value) {
    if ($init) {
      $init = FALSE;
    }
    else {
      $parameters_string = $parameters_string . '&';
    }
    $parameters_string = $parameters_string . urlencode($name) . '=' . urlencode($value);
  }
  curl_setopt($ch, CURLOPT_POSTFIELDS, $parameters_string);
  // Make the request.
  $text = curl_exec($ch);
  // Check the response for errors.
  $errnum = curl_errno($ch);
  if ($errnum != 0) {
    error_log('Commerce PayWay Net saw cURL error: '. curl_error($ch));
    watchdog('commerce_westpac_payway_net', 'cURL error getting token, Error %errnum, %errmsg', array('%errnum' => $errnum, '%errmsg' => curl_error($ch)), WATCHDOG_ERROR);
    header("HTTP/1.1 403 " . $errmsg);
    exit;
  }
  curl_close($ch);
  // Split the response into parameters.
  $rsp_parameter_array = explode("&", $text);
  $rsp_parameters = array();
  foreach ($rsp_parameter_array as $parameter) {
    list($name, $value) = explode("=", $parameter, 2);
    $rsp_parameters[$name] = $value;
  }
  if (array_key_exists('error', $rsp_parameters)) {
    watchdog('commerce_westpac_payway_net', 'Error getting token: %err', array('%err' => $rsp_parameters['error']), WATCHDOG_ERROR);
    throw new Exception(t('cURL error: %err', array('%err' => $rsp_parameters['error'])));
  }
  else {
    return $rsp_parameters['token'];
  }
}
