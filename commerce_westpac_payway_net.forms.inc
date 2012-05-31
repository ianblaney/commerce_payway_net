<?php

/**
 * EXTRA FORM SHOWN TO CUSTOMER WHEN THEY CHOOSE WESTPAC PAYWAY NET PAYMENT METHOD
 * Payment method callback: submit form.
 */
function commerce_westpac_payway_net_submit_form($payment_method, $pane_values, $checkout_pane, $order) {
  watchdog('commerce_payway_net', __FUNCTION__);
  $form = array();
  // Merge in values from the order.
  if (!empty($order->data['commerce_westpac_payway_net'])) {
    $pane_values += $order->data['commerce_westpac_payway_net'];
  }
  return $form;
}

/**
 * VALIDATION OF FORM SHOWN TO CUSTOMER WHEN THEY CHOOSE THIS PAYMENT METHOD
 * Payment method callback: submit form validation.
 */
function commerce_westpac_payway_net_submit_form_validate($payment_method, $pane_form, $pane_values, $order, $form_parents = array()) {
  watchdog('commerce_payway_net', __FUNCTION__);
}

/**
 * CALLBACK FOR FORM SHOWN TO CUSTOMER WHEN THEY CHOOSE THIS PAYMENT METHOD -> SENDS TO COMMERCE TRANSACTION
 * Payment method callback: submit form submission.
 */
function commerce_westpac_payway_net_submit_form_submit($payment_method, &$pane_form, $pane_values, $order, $charge) {
  watchdog('commerce_payway_net', __FUNCTION__);
}

/**
 * Generate form which will be used to redirect browser to PayWay.
 *
 * I'd prefer to move the drupal_goto() to $form['#redirect'] or even the hook_submit() for the previous 
 * form, but that interferes with the other submit functions. So here it is.
 */
function commerce_westpac_payway_net_redirect_form($form, &$form_state, $order, $payment_method) {
  module_load_include('inc', 'commerce_westpac_payway_net');
  watchdog('commerce_payway_net', __FUNCTION__);
  $order->data['commerce_westpac_payway_net'] = $pane_values;
  commerce_westpac_payway_net_transaction($payment_method, $order, $charge, $pane_values['name']);
  
  $return_link_url = url('checkout/' . $order->order_id . '/payment/return/' . $order->data['payment_redirect_key'], array(), array('absolute' => TRUE));
  // Commerce has price in cents, Payway wants dollars
  $payment_amount = $order->commerce_order_total['und'][0]['amount'] / 100;
  $return_link_url = url($return_link_url, array('absolute' => TRUE));
  $payway_base_url = $payment_method['settings']['commerce_westpac_payway_net_payWayBaseUrl'];
  $biller_code = $payment_method['settings']['commerce_westpac_payway_net_billerCode'];
  $token_variables = array(
    'payway_url'        => $payway_base_url,
    'payment_reference' => $order->order_id,
    'payment_amount'    => $payment_amount,
    'return_link_url'   => $return_link_url,
    'biller_code'       => $biller_code,
    'username'          => $payment_method['settings']['commerce_westpac_payway_net_username'],
    'password'          => $payment_method['settings']['commerce_westpac_payway_net_password'],
    'merchant_id'       => $payment_method['settings']['commerce_westpac_payway_net_merchantId'],
    'paypal_email'      => $payment_method['settings']['commerce_westpac_payway_net_paypalEmail'],
    'cert_path'         => $payment_method['settings']['commerce_westpac_payway_net_caCertsFile'],
  );
  //dpm($payment_method, 'payment method');
  //dpm($pane_values, 'pane values');
  //dpm($checkout_pane, 'checkout pane');
  //dpm($order, 'order');
  try {
    $token = _commerce_westpac_payway_net_get_token($token_variables);
    $url = $payway_base_url .
      'MakePayment?biller_code=' . $biller_code . 
      '&token=' . $token;
    drupal_goto($url);
  }
  catch (Exception $e) {
    form_set_error('Unable to communicate with PayWay Net payment gateway.');    
  }
}


function commerce_westpac_payway_net_redirect_form_validate($order, $payment_method) {
  watchdog('commerce_payway_net', __FUNCTION__);
}

function commerce_westpac_payway_net_redirect_form_submit($order, $payment_method) {
  watchdog('commerce_payway_net', __FUNCTION__);
}
