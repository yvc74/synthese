function mt_new_click()
{
  $('#mt_new_modal').modal('show');
  return false;
}

function mt_remove_click()
{
  $('#remove_title').html("de type de message");
  $('#remove_name').html("le type de message "+ $(this).attr('element_name'));
  $('#remove_form_tab_AUTOGENERATEDFIELDID').val('message_types');
  $('#remove_form_open_subscriptions_AUTOGENERATEDFIELDID').val(0);
  $('#remove_form_actionParamoi_AUTOGENERATEDFIELDID').val($(this).attr('element_id'));
  $('#remove_modal').modal('show');
  return false;
}

function np_new_click(manu)
{
  $('#np_new_modal').modal('show');
  return false;
}
function mtag_new_click()
{
  $('#mtag_new_modal').modal('show');
  return false;
}


function np_remove_click()
{
  $('#remove_title').html("de fournisseur de notification");
  $('#remove_name').html("le fournisseur de notification "+ $(this).attr('element_name'));
  $('#remove_form_tab_AUTOGENERATEDFIELDID').val('notification_providers');
  $('#remove_form_open_subscriptions_AUTOGENERATEDFIELDID').val(0);
  $('#remove_form_actionParamoi_AUTOGENERATEDFIELDID').val($(this).attr('element_id'));
  $('#remove_modal').modal('show');
  return false;
}

function mtag_remove_click()
{
  $('#remove_title').html("de liste de diffusion");
  $('#remove_name').html("la liste de diffusion "+ $(this).attr('element_name'));
  $('#remove_form_tab_AUTOGENERATEDFIELDID').val('mailing_lists');
  $('#remove_title').html("de mot clef pour message");
  $('#remove_name').html("le mot clef pour message "+ $(this).attr('element_name'));
  $('#remove_form_tab_AUTOGENERATEDFIELDID').val('message_tags');
  $('#remove_form_open_subscriptions_AUTOGENERATEDFIELDID').val(0);
  $('#remove_form_actionParamoi_AUTOGENERATEDFIELDID').val($(this).attr('element_id'));
  $('#remove_modal').modal('show');
  return false;
}

function bp_remove_click()
{
  $('#remove_title').html("de point de diffusion");
  $('#remove_name').html("le point de diffusion "+ $(this).attr('element_name'));
  $('#remove_form_tab_AUTOGENERATEDFIELDID').val('broadcast_points');
  $('#remove_form_open_subscriptions_AUTOGENERATEDFIELDID').val(0);
  $('#remove_form_actionParamoi_AUTOGENERATEDFIELDID').val($(this).attr('element_id'));
  $('#remove_modal').modal('show');
  return false;
}

function bp_new_click()
{
  $('#bp_new_modal').modal('show');
  return false;
}

function s_new_click()
{
  $('#s_new_form_actionParam_field_rank_AUTOGENERATEDFIELDID').val($(this).attr('element_rank'));
  $('#s_new_modal').modal('show');
  return false;
}

function s_sort(event, ui)
{
  $('#s_rank_form_actionParam_object_id_AUTOGENERATEDFIELDID').val(ui.item.attr('element_id'));
  $('#s_rank_form_actionParam_field_rank_AUTOGENERATEDFIELDID').val(ui.item.index()-1);
  $('#s_rank_form').submit();
}

function changeMediaId() {
  var field = $(this);
  $('#' + field.attr('field')).val(field.val());
}

function changeIneoTerminusNetwork() {
  var field = $(this);
  $('#' + field.attr('field')).val(field.val());
}

function changeIneoTerminusDatasource() {
  var field = $(this);
  $('#' + field.attr('field')).val(field.val());
}

function changeIneoTerminusFakeBroadcast() {
  var field = $(this);
  $('#' + field.attr('field')).val(field.val());
}

function np_select_template_click()
{
  $('.np_template_element').removeClass('active');
  $(this).addClass('active');
  $('#np_generate').prop('disabled', false);
}

function np_generate_fields()
{
  var scenarioId = $('#np_templates').find('li.np_template_element.active').attr('element_id');
  var providerId = $('#np_update_parameters_actionParam_object_id_AUTOGENERATEDFIELDID').val();
  var parameterStr = $('#np_update_parameters').find('[is_parameter=true]').serialize();
  var eventType = $('#np_generate_type').val();
  var postData = {
    'scenario_id': scenarioId,
    'notification_provider_id': providerId,
    'test_notification_type': eventType,
    'test_parameters': parameterStr
  };
  $.ajax({ type: "POST",
           async: "true",
           url: "/terminus/ajax/test_notification_provider",
           data: postData
         })
    .done(function(data) {
      $('#np_test_results').html(data);
      // Copy field labels
      $('#np_test_results').find('.test_label')
        .each(function(idx, label) {
          var fieldName = $(label).attr('field_name');
          $(label).html($('#np_update_parameters').find('#label_' + fieldName).html());
        });
    })
    .fail(function(data) {
      $('#np_test_results').html("La génération a échouée");
    });
}

$(function(){
  // Generic
  $('.modal[show_modal=1]').removeClass('fade');
  $('.modal[show_modal=1]').modal('show');
  $('.modal').on('shown', function () {  $('input:text:visible:first', this).focus(); });

  // Message types
  $('#mt_new_link').click(mt_new_click);
  $('#mt_remove_link').click(mt_remove_click);
  $('#mtag_new_link').click(mtag_new_click);
  $('#mtag_remove_link').click(mtag_remove_click);
  // Notification provider tab
  $('#np_new_link').click(np_new_click);
  $('#np_remove_link').click(np_remove_click);
  $('#np_update_parameters').submit(function() {
    var parameterStr = $(this).find('[is_parameter=true]').serialize();
    $('#np_update_parameters_actionParam_field_parameters_AUTOGENERATEDFIELDID').val(parameterStr);
  });
  $('.np_template_element').click(np_select_template_click);
  $('#np_generate').click(np_generate_fields);

  // Custom broadcast point tab
  $('#bp_remove_link').click(bp_remove_click);
  $('#bp_new_link').click(bp_new_click);
  $('div[action=s_new_link]').click(s_new_click);
  //$('#s_list').sortable({ items: 'li:not(.not_sortable)', update:s_sort });
  $('#s_list li').disableSelection();
  
  $('[field=inputMediaId]').on("change", changeMediaId);
  
  $('[field=inputIneoTerminusNetwork]').on("change", changeIneoTerminusNetwork);
  $('[field=inputIneoTerminusDatasource]').on("change", changeIneoTerminusDatasource);
  $('[field=inputIneoTerminusPassengerFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
  $('[field=inputIneoTerminusDriverFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
  $('[field=inputIneoTerminusPpdsFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
  $('[field=inputIneoTerminusGirouetteFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
  $('[field=inputIneoTerminusSonoPassengerFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
  $('[field=inputIneoTerminusSonoDriverFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
  $('[field=inputIneoTerminusSonoStopPointFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
  $('[field=inputIneoTerminusBivGeneralFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
  $('[field=inputIneoTerminusBivLineAutoFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
  $('[field=inputIneoTerminusBivLineManFakeBroadcast]').on("change", changeIneoTerminusFakeBroadcast);
});