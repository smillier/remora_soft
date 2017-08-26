/*
* Project: Remora = v1.3.4
* Description: IHM Web for Remora
* Author: Charles Henri Hallard, Thibaud, Manuel Hervo
* License: MIT License
* Website: https://github.com/hallard/remora_soft
*/

/* global define:false, require: false, jQuery:false */
var app;
+function ($) {
  "use strict";
  app = {
    settings: {
      version: "@remora_version",
      author: "remora",
      debug: false,
      elapsed: 0,
      url: '',
      full: '',
      hidden: 'hidden',
      visibilityChange: 'visibilitychange',
      visibilityState: 'visibilityState',
      template: '<div class="col-sm-6 col-md-4 zone">'
             + '  <div style="min-height:80px;" class="thumbnail" data-zone="#zone#" title="Gestion de la zone #zone#">'
             + '    <div class="caption"><h5>Zone #zone#</h5><span class="icon iconCmd" style="font-size: 3.5em;"></span>'
             + '    <div>'
             + '      <a href="#" class="actions btn btn-default conf" role="button">Confort</a>'
             + '      <a href="#" class="actions btn btn-default eco" role="button">ECO</a>'
             + '      <a href="#" class="actions btn btn-default hg" role="button">hors gel</a>'
             + '      <a href="#" class="actions btn btn-default off" role="button">arrêt</a>'
             + '    </div>'
             + '  </div>'
             + '</div>'
    },
    // Les timeouts
    timers: {
      sys: null,
      tinfo: null,
    },
    /**
     * Cette fonction sert à initialiser l'objet
     *
     * @return null
     */
    init: function() {
      var t = this,
          a = t.settings;
      // Détection préalable des préfixes pour chaque moteur
      // et stockage de leur nom dans des variables
      if (typeof document.hidden !== 'undefined') {
        a.hidden = 'hidden';
        a.visibilityChange = 'visibilitychange';
        a.visibilityState = 'visibilityState';
      } else if (typeof document.mozHidden !== 'undefined') {
        a.hidden = 'mozHidden';
        a.visibilityChange = 'mozvisibilitychange';
        a.visibilityState = 'mozVisibilityState';
      } else if (typeof document.msHidden !== 'undefined') {
        a.hidden = 'msHidden';
        a.visibilityChange = 'msvisibilitychange';
        a.visibilityState = 'msVisibilityState';
      } else if (typeof document.webkitHidden !== 'undefined') {
        a.hidden = 'webkitHidden';
        a.visibilityChange = 'webkitvisibilitychange';
        a.visibilityState = 'webkitVisibilityState';
      }
      console.log("%cInformations\nversion: " + a.version + "\nauthor: " + a.author, "color: #ae81bc");
      a.url = document.location.toString();
      a.full = location.protocol+'//'+location.hostname+(location.port ? ':'+location.port: '');
      if (a.debug) console.log('app initialized');
    },
    /**
     * Cette fonction sert à lancer l'application
     *
     * @return null
     */
    launch: function() {
      var t = this,
          a = t.settings,
          tab = 'tab_sys';

      $('.nav-tabs a').on('shown', function(e) {
        window.location.hash = e.target.hash;
      });
      if (a.url.match('#')) {
        tab = a.url.split('#')[1];
      }
      $('.nav-tabs a[href=#' + tab + ']').tab('show').trigger('shown');

      // enlever le loader, tout est prêt
      $('body').addClass('loaded');
      if (a.debug) console.log('app launched');
    },
    /**
     * Cette fonction sert à afficher une notification
     *
     * @input number mydelay Le délai d'affichage en secondes
     * @input string myicon  La classe CSS glyphicon
     * @input string mytype  Le type de notification (success, danger)
     * @input string mytitle Le titre de la bulle de notification
     * @input string mymsg   Le message de la bulle de notification
     * @return null
     */
    notify: function(mydelay, myicon, mytype, mytitle, mymsg) {
      $('body').addClass('loaded');
      $.notify(
        { icon:'glyphicon glyphicon-'+myicon,
          title:'&nbsp;<strong>'+mytitle+'</strong>',
          message:'<p>'+mymsg+'</p>',
        },{
          type:mytype,
          //showProgressbar: true,
          animate:{enter:'animated fadeInDown',exit:'animated fadeOutUp',delay:mydelay*1000}
        }
      );
    },
    /**
     * Cette fonction sert à afficher une barre de progression
     *
     * @input Object data L'objet contenant les données de progression
     * {loaded: number, total: number}
     *
     * @return null
     */
    progressUpload: function(data) {
      if (data.lengthComputable) {
        var pe = (data.loaded/data.total*100).toFixed(0) ;
        $('#pfw').css('width', pe +'%');
        $('#psfw').text(formatSize(data.loaded)+' / '+formatSize(data.total));
      }
    },
    /**
     * Cette fonction affiche une modal d'attente en attendant d'avoir
     * un retour de l'API
     *
     * @return null
     */
    waitReboot: function() {
      var t = this,
          a = t.settings,
          url = a.full + '/#tab_sys';
      $('#txt_srv').text('Tentative de connexion à ' + url);
      $('#mdl_wait').modal();

      // On appel l'API jusqu'à ce qu'elle réponde, toutes les secondes
      var thistimer = setInterval(function() {
        $.ajax({
          cache: false,
          type: 'GET',
          url: '/hb.htm',
          timeout: 900,
          success: function(data, textStatus, XMLHttpRequest) {
            if (a.debug) console.log(data);
            if (data === 'OK') {
              $('#mdl_wait').modal('hide');
              clearInterval(thistimer);
              window.location = url;
              location.reload();
              a.elapsed = 0;
            }
          }
        });
        a.elapsed++;
        $('#txt_elapsed').text('Temps écoulé ' + a.elapsed + ' s');
      }
      , 1000);
    },
    /**
     * Cette fonction sert à créer une vignette d'une zone de fil pilote
     *
     * @input number   id    L'identifiant de la zone
     * @input [string] zones Les ordres de chaque zone
     * @return null
     */
    addZoneTemplate: function(id, zones) {
      var t = this,
          a = t.settings;
      // On récupère la template, en ajoutant l'identifiant de la zone
      var template = a.template.replace(/#zone#/g, id.replace('fp', ''));
      $('#tab_fp .zones').append(template); // On ajoute la template dans le body du panel
      var $div = $('#tab_fp .zones div.zone:last'); // Dernière template
      t.activeZone(id.replace('fp', ''), zones[id]); // On active le bon bouton et l'image associée
      // On ajoute le bind sur click sur les boutons
      $('.actions', $div).on('click', function(e) {
        if (e.stopPropagation) e.stopPropagation();
        if (e.preventDefault) e.preventDefault();
        var $this = $(this),
            // On récupère l'identifiant de la zone
            zone = $this.parents('.thumbnail').data('zone');
        if ($this.hasClass('conf')) {
          t.sendOrdre(zone, 'C');
        } else if ($this.hasClass('eco')) {
          t.sendOrdre(zone, 'E');
        } else if ($this.hasClass('hg')) {
          t.sendOrdre(zone, 'H');
        } else if ($this.hasClass('off')) {
          t.sendOrdre(zone, 'A');
        }
      });
    },
    /**
     * Fonction appelant l'API pour modifier l'ordre d'une zone
     *
     * @input number id    L'identifiant de la zone
     * @input string ordre L'ordre à envoyer
     * @return null
     */
    sendOrdre: function(id, ordre) {
      var t = this,
          a = t.settings;

      $('body').removeClass('loaded'); // On active le loader
      if (a.debug) console.log('sendOrdre ', id, ordre);
      var request = {setfp: id + ordre}; // On définit l'ordre de la zone
      if (id == false) {
        // Ordre pour toutes les zones
        request = {fp: Array(8).join(ordre)};
      }
      if (a.debug) console.log('request', request);
      $.getJSON('/', request)
        .done(function(data, textStatus, jqXHR) {
          if (a.debug) console.log('response ', request, data, textStatus, jqXHR);
          // Si la réponse est OK, on met à jour l'affichage de la/des zone(s)
          if (data.hasOwnProperty('response') && data.response == 0) {
            if (id === false) {
              $('.zones .zone').each(function(index, elt) {
                var id = $('.thumbnail', elt).data('zone');
                t.activeZone(id, ordre);
              });
            } else {
              t.activeZone(id, ordre);
            }
          }
          // Sinon, on affiche une notification d'erreur
          else {
            if (a.debug) console.error('Error lors de l\'envoi d\'ordre(%s) pour fil pilote(%s):', ordre, id, data);
            t.notify(4, 'remove', 'danger', 'Error ordre '+ordre+' pour le fil pilote #'+id, jqXHR.status+' '+data);
          }
          $('body').addClass('loaded'); // On retire le loader
        }).fail(function( jqXHR, textStatus, error ) {
          var err = textStatus + ", " + error;
          if (a.debug) console.error("Request Failed: " + err);
          // On affiche une notification d'erreur
          t.notify(4, 'remove', 'danger', 'Error ordre '+ordre+' pour le fil pilote #'+id, jqXHR.status+' '+err);
          $('body').addClass('loaded'); // On retire le loader
        });
    },
    /**
     * Fonction permettant d'afficher l'état d'une zone de fil pilote
     *
     * @input number id    L'identifiant de la zone à activer
     * @input string state L'ordre de la zone
     * @return null
     */
    activeZone: function(id, state) {
      var t = this,
          a = t.settings;

      if (a.debug) console.log('activeZone', id, state);
      
      var $div = $('div.thumbnail[data-zone="'+id+'"]'),
          $icon = $('span.icon', $div),
          active,
          img;
      
      // On définit la classe active et l'icône
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
      if (a.debug) console.log('active: %s - img: %s', active, img);
      $('a.actions', $div).removeClass('active btn-success');
      // On met à jour l'affichage du bouton actif
      $(active, $div).addClass('active btn-success');
      // On remplace l'icône, si différent
      if ($icon.attr('src') != img) {
        $icon.empty();
        $icon.append('<i class="icon ' + img + '"></i>');
      }
    },
    /**
     * Fonction appelant l'API pour modifier l'état du relais
     *
     * @input string api   L'API à appeler
     * @input string ordre L'ordre à envoyer
     * @return null
     */
    sendRelais: function(api, ordre) {
      var t = this,
          a = t.settings;

      $('body').removeClass('loaded'); // On active le loader
      if (a.debug) console.log('sendRelais[%s] ', api, ordre);
      var request = {};
      request[api] = ordre;
      if (a.debug) console.log('request', request);
      $.getJSON('/', request)
        .done(function(data, textStatus, jqXHR) {
          if (a.debug) console.log('response ', request, data, textStatus, jqXHR);
          if (data.hasOwnProperty('response') && data.response == 0) {
            // On rappelle l'API pour avoir l'état du relais et du fonctionnement du relais
            $.getJSON('/relais', function(data) {
              if (data.hasOwnProperty('relais') && data.hasOwnProperty('fnct_relais')) {
                // On met à jour l'affichage du relais
                t.activeRelais(data.fnct_relais, data.relais);
              }
            });
          } else {
            if (a.debug) console.error('Error lors de l\'envoi d\'ordre(%s) pour relais:', ordre, data);
            t.notify(4, 'remove', 'danger', 'Error ordre '+ordre+' pour le relais', jqXHR.status+' '+data);
          }
          $('body').addClass('loaded'); // On retire le loader
        }).fail(function( jqXHR, textStatus, error ) {
          var err = textStatus + ", " + error;
          if (a.debug) console.error("Request Failed: " + err);
          t.notify(4, 'remove', 'danger', 'Error ordre '+ordre+' pour le relais', jqXHR.status+' '+err);
          $('body').addClass('loaded'); // On retire le loader
        });
    },
    /**
     * Fonction permettant d'afficher l'état du relais
     *
     * @input string fnct  L'état de fonctionnement du relais
     * @input string state L'état du relais
     * @return null
     */
    activeRelais: function(fnct, state) {
      var t = this,
          a = t.settings;

      if (a.debug) console.log('activeRelais', fnct, state);
      var $icon = $('#tab_fp .relais span.icon'),
          active = '#tab_fp .relais',
          img = 'jeedom-relais-on';

      if (parseInt(state, 10) == 0) {
        img = 'jeedom-relais-off';
      }
      // On définit la classe active pour le relais
      switch (parseInt(fnct, 10)) {
        case 0: active += ' a.off'; break;
        case 1: active += ' a.force'; break;
        case 2:
        default:
          active += ' a.auto';
          break;
      }
      if (a.debug) console.log('active: %s - img: %s', active, img);
      $icon.data('val', state);
      $('#tab_fp .relais a.actions').removeClass('active btn-success');
      // On met à jour le bouton actif
      $(active).addClass('active btn-success');
      // On remplace l'icône, si différent
      if ($icon.attr('src') != img) {
        $icon.empty();
        $icon.append('<i class="icon ' + img + '"></i>');
      }
    },
    /**
     * Fonction permettant de charger les données dans une table bootstrap
     *
     * @input Object $element L'élément jQuery sur lequel charger la table
     * @input string pathData L'URL de l'API à utiliser (Optionnel)
     * @return null
     */
    loadData: function($element, pathData) {
      var options = {silent: true};
      if (typeof pathData == 'string') {
        options.url = pathData;
      }
      $element.bootstrapTable('refresh', options);
    },
    /**
     * Fonction testant si la page est affichée, et recharge la table
     * d'un $element si la page est visible
     *
     * @input Object $element  L'élément sur lequel recharger les données
     * @input number timeoutID La variable de stockage du timeout
     * @input Object app       L'objet app utilisé par la boucle (Optionnel)
     * @return null
     */
    pageHidden: function($element, timeoutID, app) {
      var t = app || this,
          a = t.settings;
      // Si la page n'est pas affichée, on reteste dans 1 seconde sans appel à l'API
      if (document[a.hidden]) {
        timeoutID = setTimeout(t.pageHidden, 1000, $element, timeoutID, t);
      }
      // Sinon, on recharge le tableau
      else {
        if (a.debug) console.log('page à nouveau visible');
        timeoutID = setTimeout(t.loadData, 1000, $element);
      }
    }
  };
  app.init();

  $(function() {
    var $tabSysData = $('#tab_sys_data'),
        $tabTinfoData = $('#tab_tinfo_data'),
        $tabFsData = $('#tab_fs_data'),
        $tabScanData = $('#tab_scan_data');

    $('a[data-toggle="tab"]').on('shown.bs.tab', function (e) {
      clearTimeout(app.timers.sys);
      clearTimeout(app.timers.tinfo);
      var target = $(e.target).attr("href")
      if (app.settings.debug) console.log('activated ' + target);

      // IE10, Firefox, Chrome, etc.
      if (history.pushState)
        window.history.pushState(null, null, target);
      else
        window.location.hash = target;

      // Onglet Téléinformation
      if (target == '#tab_tinfo') {
        app.loadData($tabTinfoData, '/tinfo.json');
      }
      // Onglet Système
      else if (target == '#tab_sys') {
        app.loadData($tabSysData, '/system.json');
      }
      // Onglet Fichiers
      else if (target == '#tab_fs') {
        $.getJSON("/spiffs.json", function(spiffs_data) {
          var pb, pe, cl,
              total = spiffs_data.spiffs[0].Total,
              used = spiffs_data.spiffs[0].Used,
              freeram = spiffs_data.spiffs[0].Ram;

          $tabFsData.bootstrapTable('load', spiffs_data.files, {silent:true, showLoading:true});

          pe = parseInt(used*100/total);
          if (isNaN(pe))
            pe = 0;
          cl = 'success';
          if (pe > 70) cl = 'warning';
          if (pe > 85) cl = 'danger';

          cl = 'progress-bar-' + cl;
          if (pe > 0) {
            $('#sspiffs').text(pe+'%');
          }
          $('#fs_use').text(formatSize(used)+' / '+formatSize(total));
          $('#pfsbar').attr('class','progress-bar '+cl);
          $('#pfsbar').css('width', pe+'%');

        })
        .fail(function() { console.error( "error while requestiong spiffs data" );  })
      }
      // Onglet Configuration
      else if (target == '#tab_cfg') {
        $.getJSON( "/config.json", function(form_data) {
          $("#frm_config").autofill(form_data);
          })
          .fail(function() { console.error( "error while requestiong configuration data" ); })

        $tabScanData.bootstrapTable('refresh',{silent:true, showLoading:true, url:'/wifiscan.json'});
      }
      // Onglet Zones
      else if (target == '#tab_fp') {
        $('body').removeClass('loaded'); // On affiche le loader
        // On récupère l'état de toutes les zones
        $.getJSON('/fp', function(data) {
          $('#tab_fp .zones').empty(); // On vide l'espace d'affichage des zones
          for (var k in data) {
            app.addZoneTemplate(k, data); // On ajoute l'affichage d'une zone
          }
          // On ajoute un bind sur les boutons d'actions générales
          $('#tab_fp .all .actions').unbind('click').click(function(e) {
            if (e.stopPropagation) e.stopPropagation();
            if (e.preventDefault) e.preventDefault();
            var $this = $(this);
            if ($this.hasClass('conf')) {
              app.sendOrdre(false, 'C');
            } else if ($this.hasClass('eco')) {
              app.sendOrdre(false, 'E');
            } else if ($this.hasClass('hg')) {
              app.sendOrdre(false, 'H');
            } else if ($this.hasClass('off')) {
              app.sendOrdre(false, 'A');
            }
          });
          $('body').addClass('loaded'); // On masque le loader
        });
        // On récupère l'état du relais et du fonctionnement du relais
        $.getJSON('/relais', function(data) {
          // On met à jour l'affichage du relais
          if (data.hasOwnProperty('fnct_relais')) {
            app.activeRelais(data.fnct_relais, data.relais);
          }
          // On ajoute un bind sur l'icône du relais
          $('#tab_fp .relais .iconCmd').unbind('click').click(function(e) {
            if (e.stopPropagation) e.stopPropagation();
            if (e.preventDefault) e.preventDefault();
            var $this = $(this);
            if ($this.data('val') == 0) {
              app.sendRelais('relais', 1);
            } else {
              app.sendRelais('relais', 0);
            }
          });
          // On ajoute un bind sur les boutons du fonctionnement du relais
          $('#tab_fp .relais .actions').unbind('click').click(function(e) {
            if (e.stopPropagation) e.stopPropagation();
            if (e.preventDefault) e.preventDefault();
            var $this = $(this);
            if ($this.hasClass('force')) {
              app.sendRelais('frelais', 1);
            } else if ($this.hasClass('auto')) {
              app.sendRelais('frelais', 2);
            } else if ($this.hasClass('off')) {
              app.sendRelais('frelais', 0);
            }
          });
        });
      }
    });

    // Callbacks events sur le tableau Téléinformation
    $tabTinfoData
      .on('load-success.bs.table', function (e, data) {
        if (app.settings.debug) console.log('#tab_tinfo_data loaded', e, data);
        if ($('.nav-tabs .active > a').attr('href')=='#tab_tinfo') {
          // Si la page n'est plus affichée, on ne fait plus d'appel à l'API
          if (document[app.settings.hidden]) {
            if (app.settings.debug) console.log('page tinfo cachée');
            clearTimeout(app.timers.tinfo);
            app.pageHidden($tabTinfoData, app.timers.tinfo);
          } else {
            // Sinon, on recharge le tableau
            app.timers.tinfo = setTimeout(app.loadData, 1000, $tabTinfoData);
          }
        }
      }).on('load-error.bs.table', function (e, status, res) {
        if (app.settings.debug) console.log('Event: load-error.bs.table on tab_tinfo_data', e, status, res);
        if (status === 404 && res.hasOwnProperty('responseJSON') && res.responseJSON.hasOwnProperty('result')) {
          $('#tab_tinfo_data .no-records-found td').html("Télé-information désactivée");
        }
      });
    // Callbacks events sur le tableau Système
    $tabSysData.on('load-success.bs.table', function (e, data) {
      if (app.settings.debug) console.log('#tab_sys_data loaded');
      if ($('.nav-tabs .active > a').attr('href')=='#tab_sys') {
        // Si la page n'est plus affichée, on ne fait plus d'appel à l'API
        if (document[app.settings.hidden]) {
          if (app.settings.debug) console.log('page sys cachée');
          clearTimeout(app.timers.sys);
          app.pageHidden($tabSysData, app.timers.sys);
        } else {
          // Sinon, on recharge le tableau
          app.timers.sys = setTimeout(app.loadData, 1000, $tabSysData);
        }
      }
    });
    // Callbacks events sur le tableau Fichiers
    $tabFsData
      .on('load-success.bs.table', function (e, data) {
        if (app.settings.debug) console.log('#tab_fs_data loaded');
      })
      .on('load-error.bs.table', function (e, status, res) {
        if (app.settings.debug) console.log('Event: load-error.bs.table on tab_fs_data', e, status, res);
          // myTimer=setInterval(function(){myRefresh()},5000);
      });
    // Callbacks events sur le tableau Liste des réseaux WiFi
    $tabScanData.on('load-success.bs.table', function (e, data) {
      if (app.settings.debug) console.log('#tab_scan_data data loaded', data);
      //$(this).hide();
      if (data.status == 'OK') {
        if (app.settings.debug) console.log('#tab_scan_data: result OK');
        var networks = [];
        for (var i = 0; i < data.result.length; i++) {
          networks.push({value: data.result[i].ssid, label: data.result[i].ssid + " (RSSI: " + data.result[i].rssi + ")"});
        }
        $('#ssid').autocomplete({
          source: networks
        });
        $tabScanData.bootstrapTable('load', data.result);
      }
    });
    // On ajoute un bind sur le click d'un réseau WiFi
    $('#tab_scan').on('click-row.bs.table', function (e, name, args) {
      var $form = $('#tab_cfg');
      $('#ssid').val(name.ssid);
      setTimeout(function(){$('#psk').focus()},500);
      $('#tab_scan').modal('hide');
    });
    // On ajoute un bind sur le bouton de mise à jour des réseaux WiFi
    $('#btn_scan').click(function () {
      $tabScanData.bootstrapTable('refresh',{url:'/wifiscan.json',showLoading:false,silent:true});
    });
    $('#btn_reset').click(function () {
      $.post('/factory_reset');
      app.waitReboot();
      return false;
    });
    $('#btn_reboot').click(function () {
      $.post('/reset');
      app.waitReboot();
      return false;
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

    // Formulaire de configuration
    $('#frm_config').validator().on('submit', function (e) {
      // everything looks good!
      if (!e.isDefaultPrevented()) {
        e.preventDefault();
        if (app.settings.debug) console.log("Form Submit");

        $.post('/config_form.json', $("#frm_config").serialize())
          .done( function(msg, textStatus, xhr) {
            app.notify(2, 'ok', 'success', 'Enregistrement effectué', xhr.status+' '+msg);
          })
          .fail( function(xhr, textStatus, errorThrown) {
            app.notify(4, 'remove', 'danger', 'Erreur lors de l\'enregistrement', xhr.status+' '+errorThrown);
          }
      );
      }
    });

    // On ajoute un bind sur le champ de chargement d'un fichier Firmware
    $('#file_fw').change(function() {
      var $txt = $('#txt_upload_fw'),
          $btn = $('#btn_upload_fw'),
          ok = true,
          f = this.files[0],
          name = f.name.toLowerCase(),
          size = f.size,
          type = f.type,
          html = 'Fichier:' + name + '&nbsp;&nbsp;type:' + type + '&nbsp;&nbsp;taille:' + size + ' octets'
      if (app.settings.debug) console.log('name: ' + name + '\nsize: ' + size + '\ntype: ' + type);

      $('#pgfw').removeClass('show').addClass('hide');
      $('#sfw').text(name + ' : ');

      if (!f.type.match('application/octet-stream')) {
        app.notify(3, 'remove', 'danger', 'Type de fichier non conforme', 'Le fichier de mise à jour doit être un fichier binaire');
        ok = false;
      } else if (! /^remora_soft.*.bin$/i.test(name) ) {
        app.notify(5, 'remove', 'danger', 'Nom de fichier incorrect', 'Le fichier de mise à jour doit être nommé <ul><li>remora_soft.*.bin (Micro-logiciel) ou</li><li>remora_soft.spiffs.bin (Système de fichiers)</li></ul>');
        ok = false;
      }
      if (ok) {
        $btn.removeClass('hide');
        var label = 'Mise à jour Micro-Logiciel';
        if (name.search(/spiffs/) >= 0) {
          label = 'Mise à jour SPIFFS';
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
    // On ajoute un bind sur le bouton de chargement du firmware
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
          if(myXhr.upload)
            myXhr.upload.addEventListener('progress', app.progressUpload, false);
          return myXhr;
        },
        beforeSend: function () {
          $('#pgfw').removeClass('hide');
        },
        success: function(msg, textStatus, xhr) {
          app.notify(2, 'floppy-saved', 'success','Envoi de la mise à jour terminé', '<strong>'+xhr.status+'</strong> '+msg);
          app.waitReboot();
        },
        error: function(xhr, textStatus, errorThrown) {
          $('#pfw').removeClass('progress-bar-success').addClass('progress-bar-danger');
          app.notify(4, 'floppy-remove', 'danger', 'Erreur lors de la mise à jour du fichier '+name,'<strong>'+xhr.status+'</strong> '+errorThrown);
        }
      });
    });

    app.launch();
  });
}(jQuery);