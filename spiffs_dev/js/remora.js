!function(t){"function"==typeof define&&define.amd?define(["jquery"],t):"object"==typeof module&&module.exports?module.exports=t(require("jquery")):t(jQuery)} (function(t){var e={},a={};t.ajaxq=function(r,i){function n(t){if(e[r])e[r].push(t);else{e[r]=[];var i=t();a[r]=i}}function o(){if(e[r]){var t=e[r].shift();if(t){var i=t();a[r]=i}else delete e[r],delete a[r]}}if("undefined"==typeof i)throw"AjaxQ: queue name is not provided";var s=t.Deferred(),l=s.promise();l.success=l.done,l.error=l.fail,l.complete=l.always;var d="function"==typeof i,u=d?null:t.extend(!0,{},i);return n(function(){var e=t.ajax.apply(window,[d?i():u]);return e.done(function(){s.resolve.apply(this,arguments)}),e.fail(function(){s.reject.apply(this,arguments)}),e.always(o),e}),l},t.each(["getq","postq"],function(e,a){t[a]=function(e,r,i,n,o){return t.isFunction(i)&&(o=o||n,n=i,i=void 0),t.ajaxq(e,{type:"postq"===a?"post":"get",url:r,data:i,success:n,dataType:o})}});var r=function(t){return e.hasOwnProperty(t)&&e[t].length>0||a.hasOwnProperty(t)},i=function(){for(var t in e)if(r(t))return!0;return!1};t.ajaxq.isRunning=function(t){return t?r(t):i()},t.ajaxq.getActiveRequest=function(t){if(!t)throw"AjaxQ: queue name is required";return a[t]},t.ajaxq.abort=function(r){if(!r)throw"AjaxQ: queue name is required";var i=t.ajaxq.getActiveRequest(r);delete e[r],delete a[r],i&&i.abort()},t.ajaxq.clear=function(t){if(t)e[t]&&(e[t]=[]);else for(var a in e)e.hasOwnProperty(a)&&(e[a]=[])}}),function(t){t.fn.extend({autofill:function(e,a){var r={findbyname:!0,restrict:!0},i=this;return a&&t.extend(r,a),this.each(function(){t.each(e,function(e,a){var n,o;if(r.findbyname)n='[name="'+e+'"]',o=r.restrict?i.find(n):t(n),1==o.length?o.val("checkbox"==o.attr("type")?[a]:a):o.length>1?o.val([a]):(n='[name="'+e+'[]"]',o=r.restrict?i.find(n):t(n),o.each(function(){t(this).val(a)}));else if(n="#"+e,o=r.restrict?i.find(n):t(n),1==o.length)o.val("checkbox"==o.attr("type")?[a]:a);else{var s=!1;o=r.restrict?i.find('input:radio[name="'+e+'"]'):t('input:radio[name="'+e+'"]'),o.each(function(){s=!0,this.value==a&&(this.checked=!0)}),s||(o=r.restrict?i.find('input:checkbox[name="'+e+'[]"]'):t('input:checkbox[name="'+e+'[]"]'),o.each(function(){t(this).val(a)}))}})})}})} (jQuery),function(t){"use strict";function e(e){return this.each(function(){var r=t(this),i=t.extend({},a.DEFAULTS,r.data(),"object"==typeof e&&e),n=r.data("bs.validator");(n||"destroy"!=e)&&(n||r.data("bs.validator",n=new a(this,i)),"string"==typeof e&&n[e]())})}var a=function(e,r){this.$element=t(e),this.options=r,r.errors=t.extend({},a.DEFAULTS.errors,r.errors);for(var i in r.custom)if(!r.errors[i])throw new Error("Missing default error message for custom validator: "+i);t.extend(a.VALIDATORS,r.custom),this.$element.attr("novalidate",!0),this.toggleSubmit(),this.$element.on("input.bs.validator change.bs.validator focusout.bs.validator",t.proxy(this.validateInput,this)),this.$element.on("submit.bs.validator",t.proxy(this.onSubmit,this)),this.$element.find("[data-match]").each(function(){var e=t(this),a=e.data("match");t(a).on("input.bs.validator",function(t){e.val()&&e.trigger("input.bs.validator")})})};a.INPUT_SELECTOR=':input:not([type="submit"], button):enabled:visible',a.DEFAULTS={delay:500,html:!1,disable:!0,custom:{},errors:{match:"Does not match",minlength:"Not long enough"},feedback:{success:"glyphicon-ok",error:"glyphicon-remove"}},a.VALIDATORS={"native":function(t){var e=t[0];return e.checkValidity?e.checkValidity():!0},match:function(e){var a=e.data("match");return!e.val()||e.val()===t(a).val()},minlength:function(t){var e=t.data("minlength");return!t.val()||t.val().length>=e}},a.prototype.validateInput=function(e){var a=t(e.target),r=a.data("bs.validator.errors");if(a.is('[type="radio"]')&&(a=this.$element.find('input[name="'+a.attr("name")+'"]')),this.$element.trigger(e=t.Event("validate.bs.validator",{relatedTarget:a[0]})),!e.isDefaultPrevented()){var i=this;this.runValidators(a).done(function(n){a.data("bs.validator.errors",n),n.length?i.showErrors(a):i.clearErrors(a),r&&n.toString()===r.toString()||(e=n.length?t.Event("invalid.bs.validator",{relatedTarget:a[0],detail:n}):t.Event("valid.bs.validator",{relatedTarget:a[0],detail:r}),i.$element.trigger(e)),i.toggleSubmit(),i.$element.trigger(t.Event("validated.bs.validator",{relatedTarget:a[0]}))})}},a.prototype.runValidators=function(e){function r(t){return e.data(t+"-error")||e.data("error")||"native"==t&&e[0].validationMessage||o.errors[t]}var i=[],n=t.Deferred(),o=this.options;return e.data("bs.validator.deferred")&&e.data("bs.validator.deferred").reject(),e.data("bs.validator.deferred",n),t.each(a.VALIDATORS,t.proxy(function(t,a){if((e.data(t)||"native"==t)&&!a.call(this,e)){var n=r(t);!~i.indexOf(n)&&i.push(n)}},this)),!i.length&&e.val()&&e.data("remote")?this.defer(e,function(){var a={};a[e.attr("name")]=e.val(),t.get(e.data("remote"),a).fail(function(t,e,a){i.push(r("remote")||a)}).always(function(){n.resolve(i)})}):n.resolve(i),n.promise()},a.prototype.validate=function(){var t=this.options.delay;return this.options.delay=0,this.$element.find(a.INPUT_SELECTOR).trigger("input.bs.validator"),this.options.delay=t,this},a.prototype.showErrors=function(e){var a=this.options.html?"html":"text";this.defer(e,function(){var r=e.closest(".form-group"),i=r.find(".help-block.with-errors"),n=r.find(".form-control-feedback"),o=e.data("bs.validator.errors");o.length&&(o=t("<ul/>").addClass("list-unstyled").append(t.map(o,function(e){return t("<li/>")[a](e)})),void 0===i.data("bs.validator.originalContent")&&i.data("bs.validator.originalContent",i.html()),i.empty().append(o),r.addClass("has-error"),n.length&&n.removeClass(this.options.feedback.success)&&n.addClass(this.options.feedback.error)&&r.removeClass("has-success"))})},a.prototype.clearErrors=function(t){var e=t.closest(".form-group"),a=e.find(".help-block.with-errors"),r=e.find(".form-control-feedback");a.html(a.data("bs.validator.originalContent")),e.removeClass("has-error"),r.length&&r.removeClass(this.options.feedback.error)&&r.addClass(this.options.feedback.success)&&e.addClass("has-success")},a.prototype.hasErrors=function(){function e(){return!!(t(this).data("bs.validator.errors")||[]).length}return!!this.$element.find(a.INPUT_SELECTOR).filter(e).length},a.prototype.isIncomplete=function(){function e(){return"checkbox"===this.type?!this.checked:"radio"===this.type?!t('[name="'+this.name+'"]:checked').length:""===t.trim(this.value)}return!!this.$element.find(a.INPUT_SELECTOR).filter("[required]").filter(e).length},a.prototype.onSubmit=function(t){this.validate(),(this.isIncomplete()||this.hasErrors())&&t.preventDefault()},a.prototype.toggleSubmit=function(){if(this.options.disable){var e=t('button[type="submit"], input[type="submit"]').filter('[form="'+this.$element.attr("id")+'"]').add(this.$element.find('input[type="submit"], button[type="submit"]'));e.toggleClass("disabled",this.isIncomplete()||this.hasErrors())}},a.prototype.defer=function(e,a){return a=t.proxy(a,this),this.options.delay?(window.clearTimeout(e.data("bs.validator.timeout")),void e.data("bs.validator.timeout",window.setTimeout(a,this.options.delay))):a()},a.prototype.destroy=function(){return this.$element.removeAttr("novalidate").removeData("bs.validator").off(".bs.validator"),this.$element.find(a.INPUT_SELECTOR).off(".bs.validator").removeData(["bs.validator.errors","bs.validator.deferred"]).each(function(){var e=t(this),a=e.data("bs.validator.timeout");window.clearTimeout(a)&&e.removeData("bs.validator.timeout")}),this.$element.find(".help-block.with-errors").each(function(){var e=t(this),a=e.data("bs.validator.originalContent");e.removeData("bs.validator.originalContent").html(a)}),this.$element.find('input[type="submit"], button[type="submit"]').removeClass("disabled"),this.$element.find(".has-error").removeClass("has-error"),this};var r=t.fn.validator;t.fn.validator=e,t.fn.validator.Constructor=a,t.fn.validator.noConflict=function(){return t.fn.validator=r,this},t(window).on("load",function(){t('form[data-toggle="validator"]').each(function(){var a=t(this);e.call(a,a.data())})})} (jQuery);
var Timer_sys,
    Timer_tinfo,
    counters={},
    isousc, iinst,
    elapsed = 0,
    url,
    debug = true;

function formatSize(bytes) {
  if (bytes < 1024)  return bytes+' Bytes';
  if (bytes < (1024*1024)) return (bytes/1024).toFixed(0)+' KB';
  if (bytes < (1024*1024*1024)) return (bytes/1024/1024).toFixed(0)+' MB';
  return (bytes/1024/1024/1024).toFixed(0)+' GB';
}

function rowStyle(row, index) {
  //var classes=['active','success','info','warning','danger'];
  var flags=parseInt(row.fl,10);
  //if (flags & 0x80) return {classes:classes[4]};
  //if (flags & 0x02) return {classes:classes[3]};
  //if (flags & 0x08) return {classes:classes[1]};
  if (flags & 0x80) return {classes:'danger'};
  if (flags & 0x02) return {classes:'warning'};
  if (flags & 0x08) return {classes:'success'};
  return {};
}

function labelFormatter(value, row) {
  var flags=parseInt(row.fl,10);

  if ( typeof counters[value]==='undefined')
    counters[value]=1;
  if ( flags & 0x88 )
    counters[value]++;
  return value + ' <span class=\"badge\">'+counters[value]+'</span>';
  }

function valueFormatter(value, row) {
  if (row.na=="ISOUSC")
    isousc=parseInt(value);
  else if (row.na=="IINST") {
    var pb, pe, cl;
    iinst=parseInt(value);
    pe=parseInt(iinst*100/isousc);
    if (isNaN(pe))
      pe=0;
    cl='success';
    if (pe>70) cl ='info';
    if (pe>80) cl ='warning';
    if (pe>90) cl ='danger';

    cl = 'progress-bar-' + cl;
    if (pe>0)
      $('#scharge').text(pe+'%');
    if (typeof isousc!='undefined')
      $('#pcharge').text(iinst+'A / '+isousc+'A');
    $('#pbar').attr('class','progress-bar '+cl);
    $('#pbar').css('width', pe+'%');
  }
  return value;
}

function fileFormatter(value, row) {
  var fname = row.na;
  var htm = '';

  //ht+= '<button type="button" class="btn btn-xs btn-danger" title="Supprimer">';
  //ht+= '<span class="glyphicon glyphicon-trash" aria-hidden="true"></span>&nbsp;';
  //ht+= '</button>&nbsp;';
  //ht+= '<button type="button" class="btn btn-xs btn-primary" title="Télécharger">';
  //ht+= '<span class="glyphicon glyphicon-trash" aria-hidden="true"></span>&nbsp;';
  //ht+= '</button>&nbsp;';
  htm += '<a href="' + fname + '">' + fname + '</a>';

  return htm;
}

function RSSIFormatter(value, row) {
  var rssi = parseInt(row.rssi),
      signal = Math.min(Math.max(2*(rssi+100),0),100),
      cl;

  if (signal < 30) cl = 'danger';
  else if (signal < 50) cl = 'warning';
  else if (signal < 70) cl = 'info';
  else cl = 'success';

  cl = 'progress-bar-' + cl;
  return '<div class="progress progress-tbl">'
      + '<div class="progress-bar ' + cl + '" role="progressbar" aria-valuemin="0" aria-valuemax="100" '
      + 'aria-valuenow="' + signal + '" style="width:' + signal + '%">' + rssi + 'dB</div></div>';
}

function Notify(mydelay, myicon, mytype, mytitle, mymsg) {
  $.notify(
    { icon:'glyphicon glyphicon-' + myicon,
      title:'&nbsp;<strong>' + mytitle + '</strong>',
      message:'<p>' + mymsg + '</p>',
    },{
      type: mytype,
      //showProgressbar: true,
      animate:{enter:'animated fadeInDown', exit:'animated fadeOutUp', delay: mydelay * 1000}
    }
  );
}

function progressUpload(data) {
  if (data.lengthComputable) {
    var pe = (data.loaded / data.total * 100).toFixed(0) ;
    $('#pfw').css('width', pe + '%');
    $('#psfw').text(formatSize(data.loaded) + ' / ' + formatSize(data.total));
  }
}

function waitReboot() {
  //url = location.protocol+'//'+location.hostname+(location.port ? ':'+location.port: '') + '/#tab_sys' ;
  $('#txt_srv').text('Tentative de connexion à /#tab_sys');
  $('#mdl_wait').modal();

  var thistimer = setInterval(function() {
    $.ajax({
      cache: false,
      type: 'GET',
      url: '/hb.htm',
      timeout: 900,
      success: function(data, textStatus, XMLHttpRequest) {
        console.log(data);
        if (data === 'OK') {
          $('#mdl_wait').modal('hide');
          clearInterval(thistimer);
          window.location.replace('/#tab_sys');
          //location.reload();
          elapsed = 0;
        }
      }
    });
    elapsed++;
    $('#txt_elapsed').text('Temps écoulé ' + elapsed + ' s');
  }
  ,1000);
}

function addZoneTemplate(id, zones) {
  // console.log('addZoneTemplate ', id, zones);
  var template = '<div class="col-sm-6 col-md-4 zone">'
               + '  <div style="min-height:80px;" class="thumbnail" data-zone="#zone#" title="Gestion de la zone #zone#">'
               + '    <div class="caption"><h5>Zone #zone#</h5><span class="icon iconCmd" style="font-size: 3.5em;"></span>'
               + '    <div>'
               + '      <a href="#" class="actions btn btn-default conf" role="button">Confort</a>'
               + '      <a href="#" class="actions btn btn-default eco" role="button">ECO</a>'
               + '      <a href="#" class="actions btn btn-default hg" role="button">hors gel</a>'
               + '      <a href="#" class="actions btn btn-default off" role="button">arrêt</a>'
               + '    </div>'
               + '  </div>'
               + '</div>';
  template = template.replace(/#zone#/g, id.replace('fp', ''));
  // console.log('template: ' + template);
  // console.log('tab_fp .panel-body', $('#tab_fp .panel-body'));
  $('#tab_fp .zones').append(template); // On ajoute la template dans le body du panel
  var $div = $('#tab_fp .zones div.zone:last');
  // console.log('div.zone last', $div);
  activeZone(id.replace('fp', ''), zones[id]); // On active le bon bouton et l'image associée
  // On ajoute le bind sur click sur les boutons
  $('.actions', $div).on('click', function(e) {
    e.stopPropagation();
    e.preventDefault();
    var $this = $(this),
        zone = $this.parents('.thumbnail').data('zone');
    if ($this.hasClass('conf')) {
      sendOrdre(zone, 'C');
    } else if ($this.hasClass('eco')) {
      sendOrdre(zone, 'E');
    } else if ($this.hasClass('hg')) {
      sendOrdre(zone, 'H');
    } else if ($this.hasClass('off')) {
      sendOrdre(zone, 'A');
    }
  });
}

function sendOrdre(id, ordre) {
  $('body').removeClass('loaded');
  // console.log('sendOrdre ', id, ordre);
  var request = {setfp: id + ordre};
  if (id == false) {
    request = {fp: Array(8).join(ordre)};
  }
  // console.log('request', request);
  $.getJSON(url, request)
    .done(function(data, textStatus, jqXHR) {
      // console.log('response ', request, data, textStatus, jqXHR);
      if (data.hasOwnProperty('response') && data.response == 0) {
        if (id === false) {
          $('.zones .zone').each(function(index, elt) {
            var id = $('.thumbnail', elt).data('zone');
            activeZone(id, ordre);
          });
        } else {
          activeZone(id, ordre);
        }
      } else {
        console.error('Error lors de l\'envoi d\'ordre(%s) pour fil pilote(%s):', ordre, id, data);
      }
      $('body').addClass('loaded');
    }).fail(function( jqxhr, textStatus, error ) {
        var err = textStatus + ", " + error;
        console.error("Request Failed: " + err);
        // console.log(jqxhr);
    });
}

function activeZone(id, state) {
  // console.log('activeZone', id, state);
  var $div = $('div.thumbnail[data-zone="'+id+'"]'),
      $icon = $('span.icon', $div),
      active,
      img;
  // console.log('div.zone: ', $div);
  // console.log('icon: ', $icon);
  switch (state) {
    case 'C':
      active = 'a.conf';
      img = 'jeedom-pilote-conf';
    break;
    case 'E':
      active = 'a.eco';
      img = 'jeedom-pilote-eco';
    break;
    case 'H':
      active = 'a.hg';
      img = 'jeedom-pilote-hg';
    break;
    case 'A':
      active = 'a.off';
      img = 'jeedom-pilote-off';
    break;
  }
  // console.log('active: %s - img: %s', active, img);
  $icon.empty();
  $('a.actions', $div).removeClass('active btn-success');
  $(active, $div).addClass('active btn-success');
  $icon.append('<i class="icon ' + img + '"></i>');
}

function timestampToHuman(seconds) {
  var time = {seconds: 0, minutes: 0, hours: 0, days: 0},
      index = 0,
      divider,
      dividers = [
        {title: 'seconds', divider: 1},
        {title: 'minutes', divider: 60},
        {title: 'hours', divider: 3600},
        {title: 'days', divider: 86400}
      ],
      str_time = '';
  while (seconds > dividers[index].divider && index < dividers.length-1) {
    index++;
  }
  for (; index >= 0; index--) {
    divider = dividers[index];
    time[divider.title] = Math.floor(seconds / divider.divider);
    seconds = seconds % divider.divider;
  }
  if (time.days > 0) {
    str_time += time.days + ' day';
    if (time.days > 1) {
      str_time += 's';
    }
    str_time += ' ';
  }
  for (index = 2; index >= 0; index--) {
    divider = dividers[index];
    if (time[divider.title] < 10) {
      time[divider.title] = '0' + time[divider.title].toString();
    }
  }
  str_time += time.hours + ':' + time.minutes + ':' + time.seconds;
  return str_time;
}

var loadSysData = function loadSysData() {
  $.getJSON('/system.json', function(sys_data) {
    //console.log('loadSysData');
    if (sys_data.length > 0 && sys_data[0].na.toLowerCase() == 'uptime') {
      sys_data[0].va = timestampToHuman(sys_data[0].va);
    }
    $('#tab_sys_data').bootstrapTable('load', sys_data);
  });
};

$(document).ready(function() {
  url = document.location.toString();
  //url = 'http://192.168.1.93';
  console.log (url);
  var activeTab;

  $('a[data-toggle=\"tab\"]').on('shown.bs.tab', function (e) {
    clearTimeout(Timer_sys);
    clearTimeout(Timer_tinfo);
    var target = $(e.target).attr("href")
    if (target == activeTab) {
      return;
    }
    activeTab = target;
    console.log('activated ' + target );

    // IE10, Firefox, Chrome, etc.
    if (history.pushState) {
      window.history.pushState(null, null, target);
    }
    else {
      window.location.hash = target;
    }

    // Onglet télé-information
    if (target == '#tab_tinfo')  {
      $('#tab_tinfo_data').bootstrapTable('refresh', {silent:true, url: '/tinfo.json'});
    }
    // Onglet système
    else if (target == '#tab_sys') {
      loadSysData();
    }
    // Onglet fichiers
    else if (target == '#tab_fs') {
      $.getJSON('/spiffs.json', function(spiffs_data) {
        var pb, pe, cl;
        total= spiffs_data.spiffs[0].Total;
        used = spiffs_data.spiffs[0].Used;
        freeram = spiffs_data.spiffs[0].Ram;

        //$('#tab_fs_data').bootstrapTable({silent:true, showLoading:true});
        $('#tab_fs_data').bootstrapTable('load', spiffs_data.files, {silent:true, showLoading:true});

        pe = parseInt(used * 100 / total);
        if (isNaN(pe)) {
          pe=0;
        }
        cl = 'success';
        if (pe>70) cl = 'warning';
        if (pe>85) cl = 'danger';

        cl = 'progress-bar-' + cl;
        if (pe>0) {
          $('#sspiffs').text(pe + '%');
        }
        $('#fs_use').text(formatSize(used) + ' / ' + formatSize(total));
        $('#pfsbar').attr('class','progress-bar ' + cl);
        $('#pfsbar').css('width', pe + '%');

      })
      .fail(function() { console.log( "error while requestiong spiffs data" ); });
    }
    // Onglet de configuration
    else if (target == '#tab_cfg') {
      $.getJSON('/config.json', function(form_data) {
        $('#frm_config').autofill(form_data);
      }).fail(function() { console.log( "error while requestiong configuration data" ); });

      $('#tab_scan_data').bootstrapTable('refresh',{silent:true, showLoading:true, url: '/wifiscan.json'});
    }
    // Onglet de gestion des zones
    else if (target == '#tab_fp') {
      $('body').removeClass('loaded'); // On affiche le loader
      // On récupère l'état de toutes les zones
      $.getJSON('/fp', function(data) {
        $('#tab_fp .zones').empty(); // On vide l'espace d'affichage des zones
        for (var k in data) {
          addZoneTemplate(k, data); // On ajoute l'affichage d'une zone
        }
        $('body').addClass('loaded'); // On masque le loader
        // On ajoute un bind sur les boutons d'action généraux
        $('#tab_fp .all .actions').unbind('click').click(function(e) {
          e.stopPropagation();
          e.preventDefault();
          var $this = $(this);
          if ($this.hasClass('conf')) {
            sendOrdre(false, 'C');
          } else if ($this.hasClass('eco')) {
            sendOrdre(false, 'E');
          } else if ($this.hasClass('hg')) {
            sendOrdre(false, 'H');
          } else if ($this.hasClass('off')) {
            sendOrdre(false, 'A');
          }
        })
      });
    }
  });

  // On actualise les données de la télé-information 1 seconde après avoir affiché celles reçues
  $('#tab_tinfo_data').on('load-success.bs.table', function (e, data) {
    console.log('#tab_tinfo_data loaded');
    if ($('.nav-tabs .active > a').attr('href') == '#tab_tinfo') {
      Timer_tinfo = setTimeout(function() {
        $('#tab_tinfo_data').bootstrapTable('refresh', {silent: true})
      }, 1000);
    }
  })
  .on('load-error.bs.table', function (e, status) {
    console.error('Event: load-error.bs.table');
  });;

  // On actualise les données système 1 seconde après avoir affiché celles reçues
  $('#tab_sys_data').on('post-body.bs.table', function (e, data) {
    console.log('#tab_sys_data loaded');
    if ($('.nav-tabs .active > a').attr('href') == '#tab_sys') {
      Timer_sys = setTimeout(loadSysData, 1000);
    }
  });
  $('#tab_fs_data').on('load-success.bs.table', function (e, data) {
    console.log('#tab_fs_data loaded');
  })
  .on('load-error.bs.table', function (e, status) {
    console.error('Event: load-error.bs.table');
  });

  $('#tab_scan').on('click-row.bs.table', function (e, name, args) {
    var $form = $('#tab_cfg');
    $('#ssid').val(name.ssid);
    setTimeout(function(){ $('#psk').focus(); }, 500);
    $('#tab_scan').modal('hide');
  });
  $('#btn_scan').click(function () {
    $('#tab_scan_data').bootstrapTable('refresh', {url: '/wifiscan.json', showLoading:false, silent:true});
  });
  $('#btn_reset').click(function () {
    $.post('/factory_reset');
    waitReboot();
    return false;
  });
  $('#btn_reboot').click(function () {
    $.post('/reset');
    waitReboot();
    return false;
  });

  $('#frm_config').validator().on('submit', function (e) {
    // everything looks good!
    if (!e.isDefaultPrevented()) {
      e.preventDefault();
      console.log("Form Submit");

      $.post('/config_form.json', $('#frm_config').serialize())
        .done( function(msg, textStatus, xhr) {
          Notify(2, 'ok', 'success', 'Enregistrement effectué', xhr.status+' '+msg);
        })
        .fail( function(xhr, textStatus, errorThrown) {
          Notify(4, 'remove', 'danger', 'Erreur lors de l\'enregistrement', xhr.status + ' ' + errorThrown);
        }
      );
    }
  });

  $('#file_fw').change(function() {
    var $txt = $('#txt_upload_fw'),
        $btn = $('#btn_upload_fw'),
        ok = true,
        f = this.files[0],
        name = f.name.toLowerCase(),
        size = f.size,
        type = f.type,
        html = 'Fichier:' + name + '&nbsp;&nbsp;type:' + type + '&nbsp;&nbsp;taille:' + size + ' octets';

    console.log('name: ' + name);
    console.log('size: ' + size);
    console.log('type: ' + type);

    $('#pgfw').removeClass('show').addClass('hide');
    $('#sfw').text(name + ' : ');

    if (!f.type.match('application/octet-stream')) {
      Notify(3, 'remove', 'danger', 'Type de fichier non conforme', 'Le fichier de mise à jour doit être un fichier binaire');
      ok = false;
    //} else if (name!="remora_soft.cpp.bin" && name!="remora_soft.spiffs.bin") {
    } else if (! /^remora_soft.*.bin$/i.test(name) ) {
      Notify(5, 'remove', 'danger', 'Nom de fichier incorrect', 'Le fichier de mise à jour doit être nommé <ul><li>remora_soft.*.bin (Micro-logiciel) ou</li><li>remora_soft.spiffs.bin (Système de fichiers)</li></ul>');
      ok = false;
    }
    if (ok) {
      $btn.removeClass('hide');
      if (name === "remora_soft.spiffs.bin") {
        label = 'Mise à jour SPIFFS';
      } else {
        label = 'Mise à jour Micro-Logiciel';
      }
      $btn.val(label);
      $('#fw_info').html('<strong>' + label + '</strong> ' + html);
    } else {
      $txt.attr('readonly', '');
      $txt.val('');
      $txt.attr('readonly', 'readonly');
      $btn.addClass('hide');
    }
    return ok;
  });

  $('#btn_upload_fw').click(function() {
    var formData = new FormData($('#frm_fw')[0]);
    $.ajax({
      url: '/update',
      type: 'POST',
      data: formData,
      cache: false,
      contentType: false,
      processData: false,
      xhr: function() {
        var myXhr = $.ajaxSettings.xhr();
        if (myXhr.upload) {
          myXhr.upload.addEventListener('progress', progressUpload, false);
        }
        return myXhr;
      },
      beforeSend: function () {
        $('#pgfw').removeClass('hide');
      },
      success: function(msg, textStatus, xhr) {
        Notify(2, 'floppy-saved', 'success','Envoi de la mise à jour terminé', '<strong>'+xhr.status+'</strong> '+msg);
        waitReboot();
      },
      error: function(xhr, textStatus, errorThrown) {
        $('#pfw').removeClass('progress-bar-success').addClass('progress-bar-danger');
        Notify(4, 'floppy-remove', 'danger',
          'Erreur lors de la mise à jour du fichier ' + name, '<strong>' + xhr.status + '</strong> ' + errorThrown
        );
      }
    });
  });

  $(document)
    .on('change', '.btn-file :file', function() {
      var input = $(this),
      numFiles = input.get(0).files ? input.get(0).files.length : 1,
      label = input.val().replace(/\\/g, '/').replace(/.*\//, '');
      input.trigger('fileselect', [numFiles, label]);
    })
    .on('show.bs.collapse', '.panel-collapse', function () {
        var $span = $(this).parents('.panel').find('span.pull-right.glyphicon');
        $span.removeClass('glyphicon-chevron-down').addClass('glyphicon-chevron-up');
    })
    .on('hide.bs.collapse', '.panel-collapse', function () {
        var $span = $(this).parents('.panel').find('span.pull-right.glyphicon');
        $span.removeClass('glyphicon-chevron-up').addClass('glyphicon-chevron-down');
    });
  ;

  var tag = 'tab_tinfo';
  if (location.toString().match('#')) {
    tag = location.toString().split('#')[1];
  }
  $('.nav-tabs a[href="#' + tag + '"]').tab('show');
  $('.nav-tabs a[href="#' + tag + '"]').trigger('shown.bs.tab');
  $('.nav-tabs a[href="#' + tag + '"]').on('shown', function(e) {
    window.location.hash = e.target.hash;
  });
});