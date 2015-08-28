function new_event_click(){
  $('#new_event_modal').modal('show');
  return false;
}

function copy_event_click(){
  $('#event_copy_actionParammt_AUTOGENERATEDFIELDID').val($(this).attr('element_id'));
  $('#copy_event_name_field').val('Copie de '+ $(this).attr('element_name'));
  $('#copy_event_modal').modal('show');
  return false;
}


function select_template_click()
{
  $('.template_element').removeClass('active');
  $(this).addClass('active');
  $('#scenario_add_actionParamtpl_AUTOGENERATEDFIELDID').val($(this).attr('element_id'));
}

// DEBUT AJOUT TERMINUS 2.2

// Met en forme, pour un recipient donne (line, stoparea ou displayscreen), le contenu des destinataires d'un evenement
function compute_recipient_view(recipient, event, id, cat)
{
  var links = [];
  if(event != undefined)
  {
    if(event.calendar != undefined)
    {
      for(var i=0; i<event.calendar.length; ++i)
      {
        if (event.calendar[i].message != undefined)
        {
          for(var j=0; j<event.calendar[i].message.length; ++j)
          {
            var current_recipient=event.calendar[i].message[j][recipient+'_recipient'];
            // Cas ou le recipient est vide : Tous pour line et stoparea, Aucun pour displayscreen
            if(current_recipient.length == 0 && recipient != 'displayscreen') {
              links.push({'id': "", 'parameter': "", 'recipient_id': "0"});
            }
            else {
              for(var k=0; k<current_recipient.length; ++k)
              {
                links.push(current_recipient[k]);
              }
            }
          } 
        }
      }
    }
  }
  var s='';
  if(links.length)
  {
    for(var i=0; i<links.length; ++i)
    {
      var link = links[i];
      // Si un link correspond a "Tous" (cas des lignes et arrets, recipient_id==0 ne peut etre verifie 
      // dans le cas de displayscreen du fait qu'il n'est pas sauve dans ce cas la), 
      // il prend le dessus sur les autres (qui ne se retrouvent plus dans le resultat)
      if(link.recipient_id == 0)
      {
        s = "Tous";
        break;
      }
      else 
      {
        if(s.length) {
          s += '<br />';
        }
        if(link.parameter)
        {
          s += $('input[factory="'+recipient+'"][value="'+ link.recipient_id +'"][parameter="'+ link.parameter +'"]').closest('label').text();
        }
        else
        {
          var input = $('input[factory="'+recipient+'"][value="'+ link.recipient_id +'"][noparam]');
          s += input.closest('label').text();
        }
      }
    }
  }
  else
  {
    s = (recipient == 'displayscreen' ? "Aucun" : "Tous");
  }
  return s;
}



function clear_event()
{
  var id = $(this).attr('data-sid');
  var cat = $(this).attr('data-cat');
  $('#mRecipientsList-'+id+'-'+cat).modal('hide');
}


function match_filter(scenario_id, recipient)
{
  var event = theevent[scenario_id];
  if(event  != undefined)
  {
    if(event.calendar != undefined)
    {
      for(var i=0; i<event.calendar.length; ++i)
      {
        if (event.calendar[i].message != undefined)
        {
          for(var j=0; j<event.calendar[i].message.length; ++j)
          {
            var current_recipient=event.calendar[i].message[j][recipient];
            // Si la liste des destinataires (line ou stoparea) d'un message est vide, alors considere que le message
            // est diffuse sur TOUS (ce n'est pas le cas du displayscreen ou le message est diffuse sur AUCUN)
            if(current_recipient.length == 0 && recipient != 'displayscreen_recipient') {
              theevent[scenario_id].filtered = 1;
              break;
            }
            else {
              for(var k=0; k<current_recipient.length; ++k)
              {
                for(var z = 0; z < filters[recipient].length; z++)
                {
                  // Si l'id du destinataire dans le filtre est egal a celui du destinataire dans le msg
                  if(filters[recipient][z].recipient_id == current_recipient[k].recipient_id)
                  {
                    if(filters[recipient][z].parameter != undefined)
                    {
                      if(filters[recipient][z].parameter == current_recipient[k].parameter) {
                        theevent[scenario_id].filtered = 1;
                        break;
                      }
                    }
                    else if (current_recipient[k].parameter == "") {
                      theevent[scenario_id].filtered = 1;
                      break;
                    }
                  }
                }
                for(var z = 0; z < hiddenfilters[recipient].length; z++)
                {
                  if(hiddenfilters[recipient][z].recipient_id == current_recipient[k].recipient_id) 
                  {
                    if(hiddenfilters[recipient][z].parameter != undefined)
                    {
                      if(hiddenfilters[recipient][z].parameter == current_recipient[k].parameter)
                      {
                        theevent[scenario_id].filtered = 1;
                        break;
                      }
                    }
                    else if (current_recipient[k].parameter == "")
                    {
                      theevent[scenario_id].filtered = 1;
                      break;
                    }
                  }
                }
              }
            }
          } 
        }
      }
    }
  }
}

function clear_filters()
{
  if(!$('#clearFilters').hasClass('disabled'))
  {
    filters = {
     line_recipient:[],
     stoparea_recipient:[],
     displayscreen_recipient:[]
    };
    hiddenfilters = {
      line_recipient:[],
      stoparea_recipient:[],
      displayscreen_recipient:[]
    };
    update_filters_preview('line', filters);
    update_filters_preview('stoparea', filters);
    update_filters_preview('displayscreen', filters);
    $('#scenarios_on_tab').removeClass('hide');
    var s = '';
    $('#scenarios_filt').html(s);
    $('#scenarios_filt').addClass('hide');
  }
  $('#clearFilters').addClass('disabled');
  $('#launchFilter').addClass('disabled');
  $('#allSectionLaunchFilter').addClass('disabled');
}

function activate_filter_simple()
{
   if(!$('#launchFilter').hasClass('disabled'))
   {
     activate_filter(0);
   }
}

function activate_filter_all()
{
   if(!$('#allSectionLaunchFilter').hasClass('disabled'))
   {
     activate_filter(1);
   }
}

function activate_filter(isOnAllSections)
{
  var s = '';
  s += '<table class="table table-striped sortable">';
  s += '<thead><tr><th data-defaultsort="disabled"></th><th data-dateformat="DD-MM-YYYY hh:mm">début</th><th data-dateformat="DD-MM-YYYY hh:mm">fin</th><th data-defaultsort="asc">nom</th><th data-defaultsort="disabled">destinataires</th><th data-defaultsort="disabled">statut</th>';
  if(isOnAllSections == 0) {
    s += '<th data-defaultsort="disabled">archivage</th><th></th></tr></thead>';
  }
  else {
    s += '<th>sections</th></tr></thead>';
  }
    $('#scenarios_on_tab').addClass('hide');
    s += "<tbody>";
    
  var use_and_filter = ($('#filter_operator').val() == 0);
  for(var i in theevent)
  {
    var matchedAndFilter = true;
    var matchedOrFilter = true;
    theevent[i].filtered = 0;
    if (filters['displayscreen_recipient'].length > 0)
    {
        match_filter(i, 'displayscreen_recipient');
        if (theevent[i].filtered == 0) {
            matchedAndFilter = false;
        } else {
            matchedOrFilter = true;
        }
    }
    theevent[i].filtered = 0;
    if (filters['line_recipient'].length > 0)
    {
        match_filter(i, 'line_recipient');
        if (theevent[i].filtered == 0) {
            matchedAndFilter = false;
        } else {
            matchedOrFilter = true;
        }        
    }
    theevent[i].filtered = 0;
    if (filters['stoparea_recipient'].length > 0)
    {
        match_filter(i, 'stoparea_recipient');
        if (theevent[i].filtered == 0) {
            matchedAndFilter = false;
        } else {
            matchedOrFilter = true;
        }
    }
      
    if (use_and_filter) {
        theevent[i].filtered = matchedAndFilter ? 1 : 0;
    } else {
        theevent[i].filtered = matchedOrFilter ? 1 : 0;          
    }

    if(theevent[i].filtered == 1)
    {
      var dates = [];
      var min = null;
      var max = null;
      s += '<tr>';
      // Bouton ouvrir
      s += '<td>';
      // Si l utilisateur actuel peut lire l evenement
      if(isOnAllSections == 1) {
        // on prend la premiere section pour lequel l'utilisateur a les droits en lecture
        if(theevent[i].section != undefined) {
          if(theevent[i].section.length == 0) {
            s += '<a href="/terminus/events/event?roid=' + theevent[i].roid + '&amp;section=" class="btn btn-primary btn-mini btn-with-margins">Ouvrir</a>';
          }
          else {
            for(var n = 0; n < theevent[i].section.length; ++n)
            {
              if(theevent[i].section[n].reading_right == 1) {
                s += '<a href="/terminus/events/event?roid=' + theevent[i].roid + '&amp;section=' + theevent[i].section[n].id + '" class="btn btn-primary btn-mini btn-with-margins">Ouvrir</a>';
                break;
              }
            }
          }
        }
      }
      else if (theevent[i].current_section != undefined) {
        s += '<a href="/terminus/events/event?roid=' + theevent[i].roid + '&amp;section=' + theevent[i].current_section + '" class="btn btn-primary btn-mini btn-with-margins">Ouvrir</a>';
      }
      s += '</td>';
      if(theevent[i].calendar != undefined)
      {
        for(var j=0; j<theevent[i].calendar.length; ++j)
        {
          if (theevent[i].calendar[j].application_period != undefined)
          {
            for(var k=0; k<theevent[i].calendar[j].application_period.length; ++k)
            {
              if (theevent[i].calendar[j].application_period[k].start_date != undefined && theevent[i].calendar[j].application_period[k].start_date != "") {
                dates.push(Date.parse(theevent[i].calendar[j].application_period[k].start_date.replace(" ","T")));
              }
              if (theevent[i].calendar[j].application_period[k].end_date != undefined && theevent[i].calendar[j].application_period[k].end_date != "") {
                dates.push(Date.parse(theevent[i].calendar[j].application_period[k].end_date.replace(" ","T")));
              }
              if (dates.length > 0) {
                min = new Date(Math.min.apply(null, dates));
                max = new Date(Math.max.apply(null, dates));
              }
            }
          }
         }
       }
       // Date debut publication
       s += '<td>';
       if(min != undefined){
         s += ('0' + min.getDate()).slice(-2) + '-' + ('0' + (min.getMonth()+1)).slice(-2) + '-' +  min.getFullYear() + ' ' + ('0' + min.getHours()).slice(-2) + ':' + ('0' + min.getMinutes()).slice(-2);
       }
       s += '</td>';
       // Date fin publication
       s += '<td>';
       if(max != undefined){
         s += ('0' + max.getDate()).slice(-2) + '-' + ('0' + (max.getMonth()+1)).slice(-2) + '-' +  max.getFullYear() + ' ' + ('0' + max.getHours()).slice(-2) + ':' + ('0' + max.getMinutes()).slice(-2);
       }
       s += '</td>';
       // Nom Evenement
       s += '<td>';
       s += theevent[i].name;
       s += '</td>';
       // Apercu des destinataires
       s += '<td>';
       s += '<a href="#mRecipientsList-'+ theevent[i].roid +'-filt" data-toggle="modal" data-type="show_recipients" data-sid="'+theevent[i].roid+'" data-cat="filt">Aperçu des destinataires</a>';
       s += '<div id="mRecipientsList-' + theevent[i].scenario_id + '-filt" class="modal hide fade">';
       s += '<div class="modal-header">';
       s += '<button type="button" class="close" data-dismiss="modal" aria-hidden="true" data-type="close_recipients">\&times;</button>';
       s += '<h3>Destinataires pour l\'événement </h3>';
       s += '</div>';
       s += '<div class="modal-body">';

       //s += '<a href="/terminus/recipients_list?scenario_id=' + theevent[i].scenario_id + '&cat=filt></a>';
       // debut
       s += '<table class="table table-condensed">';
       s += '<thead>';
       s += '<tr>';
       s += '<th>rubrique</th>';
       s += '<th>destinataires</th>';
       s += '</thead>';
       s += '<tbody>';
       s += '<tr>';
       s += '<td>Lignes</td>';
       s += '<td id="view_recipients_line-' + theevent[i].scenario_id + '-filt">' + 
               compute_recipient_view('line', theevent[i], i, 'filt') + '</td>';
       s += '</tr>';
       s += '<tr>';
       s += '<td>Arrêts</td>';
       s += '<td id="view_recipients_stoparea-' + theevent[i].scenario_id + '-filt">' + 
               compute_recipient_view('stoparea', theevent[i], i, 'filt') + '</td>';
       s += '</tr>';
       s += '<tr>';
       s += '<td>Diffuseurs</td>';
       s += '<td id="view_recipients_displayscreen-' + theevent[i].scenario_id + '-filt">' + 
               compute_recipient_view('displayscreen', theevent[i], i, 'filt') + '</td>';
       s += '</tr>';
       s += '</tbody>';
       s += '</table>';
       // fin

       s += '</div>';
       s += '<div class="modal-footer">';
       s += '<a href="#" class="btn btn-primary" data-dismiss="modal" data-type="close_recipients">Fermer</a>';
       s += '</div>';
       s += '</div>';
       s += '</td>';
       // Activation ou desactivation Evenement
       s += '<td>';
       
       if(theevent[i].write_right_section == 1) {
         if (theevent[i].active == 1) {
           if ((theevent[i].belongs_to_an_auto_section == 1) && (theevent[i].manual_override == 0)) {
             s += '<div class="label label-info">Auto</div>';
           } else {
             s += '<div class="label label-success">Actif</div>';
           }
           if(isOnAllSections == 1) {
             s += '<a href="#" onclick="if (window.confirm(\'Etes-vous sûr de vouloir suspendre la diffusion de cet événement ?\')) window.location=\'/terminus/all_sections?a=scenario%5fsave&amp;actionParamena=0&amp;actionParamsid=' + 
                theevent[i].roid + '&amp;section=\';return false;" class="btn btn-warning btn-mini btn-inactive"><i class="icon-pause icon-white"></i></a>';
           } else {
             s += '<a href="#" onclick="if (window.confirm(\'Etes-vous sûr de vouloir suspendre la diffusion de cet événement ?\')) window.location=\'/terminus/events?a=scenario%5fsave&amp;actionParamena=0&amp;actionParamsid=' + 
                theevent[i].roid + '&amp;section=' + theevent[i].current_section + '\';return false;" class="btn btn-warning btn-mini btn-inactive"><i class="icon-pause icon-white"></i></a>';
           }
         }
         else {
           if ((theevent[i].belongs_to_an_auto_section == 1) && (theevent[i].manual_override == 0)) {
             s += '<div class="label label-info">Auto</div>';
           } else {
             s += '<div class="label">Inactif</div>';
           }
           if(isOnAllSections == 1) {
             s += '<a href="#" onclick="if (window.confirm(\'Etes-vous sûr de vouloir activer la diffusion de cet événement ?\')) window.location=\'/terminus/all_sections?a=scenario%5fsave&amp;actionParamena=0&amp;actionParamsid=' + 
                theevent[i].roid + '&amp;section=\';return false;" class="btn btn-warning btn-mini btn-inactive"><i class="icon-play icon-white"></i></a>';
           } else {
             s += '<a href="#" onclick="if (window.confirm(\'Etes-vous sûr de vouloir activer la diffusion de cet événement ?\')) window.location=\'/terminus/events?a=scenario%5fsave&amp;actionParamena=0&amp;actionParamsid=' + 
                theevent[i].roid + '&amp;section=\';return false;" class="btn btn-warning btn-mini btn-inactive"><i class="icon-play icon-white"></i></a>';
           }
         }
       }
       else {
         if (theevent[i].active == 1){
           if ((theevent[i].belongs_to_an_auto_section == 1) && (theevent[i].manual_override == 0)) {
             s += '<span class="label label-info">Auto</span>';
           } else {
             s += '<span class="label label-success">Actif</span>';
           }
         } else {
           if ((theevent[i].belongs_to_an_auto_section == 1) && (theevent[i].manual_override == 0)) {
             s += '<span class="label label-info">Auto</span>';
           } else {
             s += '<span class="label">Inactif</span>';
           }
         }
       }
       // Notification events flags
       if(theevent[i].hold_count > 0) {
         s += '<i class="icon-flag"></i>';
       }
       if(theevent[i].failed_count > 0) {
         s += '<i class="icon-warning-sign"></i>';
       }
       s += '</td>';
       // Si onglet de section, archivage de l evenement, sinon (onglet Toutes sections), affichage des sections de l evenement
       s += '<td>';
       if(isOnAllSections == 0) {
         if(theevent[i].archiving_right_section == 1) {
           s += '<a href="#" onclick="if (window.confirm(\'Etes-vous sûr de vouloir clore et archiver cet événement ?\')) window.location=\'/terminus/events?a=scenariostop&amp;actionParams=' + theevent[i].roid 
                 + '&amp;actionParam_archive=1'
                 + '&amp;section=' + theevent[i].current_section + '\';return false;" class="btn btn-danger btn-mini btn-with-margins"><i class="icon-eject icon-white"></i></a>';
         }
         else {
           s += 'archivage non autorisé';
         }
       }
       else {
         if(theevent[i].section != undefined)
         {
           for(var t = 0; t < theevent[i].section.length; ++t)
           {
             s += theevent[i].section[t].name;
             if(t+1 < theevent[i].section.length) {
               s += ' - ';
             }
           }
         }
       }
       s += '</td>';
       // Si onglet de section, affichage du bouton de duplication si autorise
       if(isOnAllSections == 0) {
         s += '<td>';
         s += '<a href="#" class="btn btn-danger btn-with-margins btn-mini copy_event_link" element_id="' + theevent[i].scenario_id + '" element_name=' + theevent[i].name + '>Dupliquer</a>';
         s += '</td>';
       }
       s += '</tr>';
    }
  }
  s += '</tbody>';
  s += '</table>';
  $('#scenarios_filt').html(s);
    $('#scenarios_filt').removeClass('hide');
    $.bootstrapSortable(true);
}
// FIN AJOUT TERMINUS 2.2


$(function(){
  $('#new_event_link').click(new_event_click);
  $('.copy_event_link').click(copy_event_click);
  $('.template_element').click(select_template_click);
// DEB AJOUT TERMINUS 2.2
//  $('.openclose').click(openclose);
//  $('input[factory]').change(change_filter_recipient);
//  $('[data-type=show_recipients]').click(print_recipients);
  $('[data-type=hide_recipients]').click(clear_event);
  $('#clearFilters').click(clear_filters);
  $('#launchFilter').click(activate_filter_simple);
  $('#allSectionLaunchFilter').click(activate_filter_all);
//  $('#mFilters a[factory]').click(show_recipients_click);
// FIN AJOUT TERMINUS 2.2
});
