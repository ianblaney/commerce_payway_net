<?php

/**
 * Notify handler.
 *
 * Receives POST from PayWay Net with transaction details (encrypted).
 */
function commerce_westpac_payway_net_notify_handler() {
  error_log(__FUNCTION__);
  error_log('POST: '.print_r($_POST,1));
  error_log('REQUEST: '.print_r($_REQUEST,1));
  error_log('ARGS: '.print_r(func_get_args(),1));
  parse_str($_SERVER['QUERY_STRING'], $data);
  
  watchdog('commerce_payway_net', __FUNCTION__);
  watchdog('commerce_payway_net', __LINE__);  
  watchdog('commerce_payway_net', 'Post: <pre>!dbg</pre>', array('!dbg' => print_r($_POST,1)), WATCHDOG_DEBUG);
  watchdog('commerce_payway_net', __LINE__);
  watchdog('commerce_payway_net', 'Request: <pre>!dbg</pre>', array('!dbg' => print_r($_REQUEST,1)), WATCHDOG_DEBUG);
  watchdog('commerce_payway_net', __LINE__);
  $args = func_get_args();
  watchdog('commerce_payway_net', 'Args: <pre>!dbg</pre>', array('!dbg' => print_r($args,1)), WATCHDOG_DEBUG);
  watchdog('commerce_payway_net', __LINE__);

  // Get Order associated with this Transaction
  if (empty($_POST['payment_reference'])) {
    // no payment reference supplied,
    // return access denied & log error
    watchdog('commerce_payway_net', 'Payment notification lacks order reference.', array(), WATCHDOG_NOTICE);
    drupal_access_denied();
  }
  if (!$order = commerce_order_load_by_number($_POST['payment_reference'])) {
    // no transaction found for supplied reference
    // return access denied & log error
    watchdog('commerce_payway_net', 'Unable to locate order @ref matching payment notification data.', array('@ref' => $_POST['payment_reference']), WATCHDOG_NOTICE);
    drupal_access_denied();
  }

  watchdog('commerce_payway_net', 'Order: <pre>!order</pre>', array('!order' => print_r($order,1)), WATCHDOG_DEBUG);

  // Get Payment Method associated with Order
  if (!isset($order->data['payment_method'])) {
    // no payment method on order
    // return access denied & log error
    watchdog('commerce_payway_net', 'Unable to locate payment method for order @ref.', array('@ref' => $_POST['payment_reference']), WATCHDOG_ERROR);
    drupal_access_denied();
  }
  if (!$method = commerce_payment_method_instance_load($order->data['payment_method'])) {
    // no payment method on order
    watchdog('commerce_payway_net', 'Unable to load payment method for order @ref.', array('@ref' => $_POST['payment_reference']), WATCHDOG_ERROR);
    drupal_access_denied();
  }
  
  watchdog('commerce_payway_net', 'Method: <pre>!method</pre>', array('!method' => print_r($payment_method,1)), WATCHDOG_DEBUG);

  // Authenticate notification using user/pass from payment method
  if (!isset($method['settings']['commerce_westpac_payway_net_username']) || !isset($method['settings']['commerce_westpac_payway_net_password'])) {
    // no auth settings to check
    watchdog('commerce_payway_net', 'Authentication not configured for payment method @id.', array('@id' => $method['instance_id']), WATCHDOG_ERROR);
    drupal_access_denied();
  }
  if ($_POST['username'] != $method['settings']['commerce_westpac_payway_net_username'] || $_POST['password'] != $method['settings']['commerce_westpac_payway_net_password']) {
    // no auth settings to check
    watchdog('commerce_payway_net', 'Incorrect auth data submitted against order @ref.', array('@ref' => $_POST['payment_reference']), WATCHDOG_ERROR);
    drupal_access_denied();
  }

  // Unsure why PayWay Net returns the amount with a thousands separator,
  // but there you have it.
  $_POST['am_payment'] = str_replace(',', '', $_POST['am_payment']);

  watchdog('commerce_westpac_payway_net', 'Order (notify): <pre>!order</pre>', array('!order' => print_r($order,1)), WATCHDOG_DEBUG);
  if ($_POST['fl_success'] == '1') {
    $transaction = commerce_payment_transaction_new('commerce_westpac_payway_net', $order->order_id);
    $transaction->instance_id = $method['instance_id'];
    $transaction->remote_id = $_POST['id_batch_entry'];
    $transaction->amount = $_POST['am_payment'] * 100;
    $transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
    $transaction->message = 'Paid via PayWay Net.';
    commerce_payment_transaction_save($transaction);
  }
  else {
    watchdog('commerce_payway_net', 'Payment failure reported by PayWay Net for order @order_id', array('@order_id' => $order_id), WATCHDOG_ERROR);
    $transaction = commerce_payment_transaction_new('commerce_westpac_payway_net', $order->order_id);
    $transaction->instance_id = $method['instance_id'];
    $transaction->remote_id = $_POST['id_batch_entry'];
    $transaction->amount = $_POST['am_payment'] * 100;
    $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
    $transaction->message = t('Payment via PayWay Net failed with error code @code.', array('@code' => $_POST['cd_response']));
    commerce_payment_transaction_save($transaction);
  }
}