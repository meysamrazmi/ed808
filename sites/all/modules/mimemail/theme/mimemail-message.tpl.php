<?php

/**
 * @file
 * Default theme implementation to format an HTML mail.
 *
 * Copy this file in your default theme folder to create a custom themed mail.
 * Rename it to mimemail-message--[module]--[key].tpl.php to override it for a
 * specific mail.
 *
 * Available variables:
 * - $recipient: The recipient of the message
 * - $subject: The message subject
 * - $body: The message body
 * - $css: Internal style sheets
 * - $module: The sending module
 * - $key: The message identifier
 *
 * @see template_preprocess_mimemail_message()
 */
?>
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <?php if ($css): ?>
    <style type="text/css">
      <!--
      <?php print $css ?>
      -->
    </style>
    <?php endif; ?>
  </head>
  <body id="mimemail-body" <?php if ($module && $key): print 'class="'. $module .'-'. $key .'"'; endif; ?>>
    <div id="center">
        <div id="header-mail">
            <a href="https://ed808.com"  target="_blank">
                <img src="https://ed808.com/images/logo.png" alt="">
            </a>
        </div>
        <div id="main">
          <?php print $body ?>
        </div>
      <div id="footer-mail">
<!--            <div class="first-row">-->
<!--                <a href="https://civil808.com"  target="_blank">موسسه آموزشی و مهندسی ۸۰۸</a>-->
<!--            </div>-->
<!--            <div style="margin: 23px;color:#333;">-->
<!--                <span style="line-height: 28px;" >تهران ، پل گیشا ، ابتدای فروزانفر ، جنب بانک صادرات، پلاک 1، واحد 7</span><span style="line-height: 28px;"> ، </span><span style="line-height: 28px;">تلفن : 88272694 - 021</span>-->
<!--            </div>-->
            <div>
                <div class="socialss">
                    <a class="linkedin"  href="https://www.linkedin.com/company/ed808" target="_blank">
                        <img src="https://civil808.com/sites/all/themes/sara/images/socials/linked.png">
                    </a>
                    <a class="instagram" href="https://www.instagram.com/ed808.ltd/" target="_blank">
                        <img src="https://civil808.com/sites/all/themes/sara/images/socials/insta.png">
                    </a>
                    <a class="facebook" href="https://www.facebook.com/ED808.ltd" target="_blank">
                        <img style="width: 11px;" src="https://civil808.com/sites/all/themes/sara/images/socials/facebook.png">
                    </a>
                    <a class="telegram" href="http://telegram.me/ed808" target="_blank">
                        <img src="https://civil808.com/sites/all/themes/sara/images/socials/telegram-mail.png" width="26" height="26">
                    </a>
                </div>
            </div>
<!--            <div class="las-row">-->
<!---->
<!--            </div>-->
        </div>
    </div>
  </body>
</html>
