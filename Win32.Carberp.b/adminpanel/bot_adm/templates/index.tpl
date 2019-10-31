<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="resourse-type" content="document" />
<meta name="document-state" content="dynamic" />
<title>{$title}</title>
<link href="/css/style.css" rel="stylesheet" type="text/css" media="all" charset="utf-8" />
<link href="/css/{$Cur.to}.css" rel="stylesheet" type="text/css" />
<link href="/css/window.css" rel="stylesheet" type="text/css" />
{$javascript_begin}
<script type="text/javascript" src="/js/fullajax/fullajax.js"></script>
<script type="text/javascript" src="/js/language.{$config.lang}.js"></script>
<script type="text/javascript" src="/js/core.js"></script>
<script type="text/javascript" src="/js/dragresize.js"></script>
<script type="text/javascript" src="/js/window.js"></script>
{$javascript_end}
<script type="text/javascript" src="/js/{$Cur.to}.js"></script>
</head>
<body{if $body ne ''} {$body}{/if}>
<noscript><div id="noscript">{$lang.nojs}<br />{$lang.nojs1}</div></noscript>
<div id="all">
<div id="logo">
<div id="account_top">
{if $_SESSION.user->PHPSESSID}
<br />{$lang.hello}<a href="/accounts/profile.html">{$_SESSION.user->login}</a>!<br /><br />
<!--
<a href="/accounts/settings.html"><img src="/images/modules/accounts/usrcfg.png" alt="Настройки" /></a> <a href="/accounts/rights.html"><img src="/images/modules/accounts/usrrights.png" alt="Права доступа" /></a> <a href="/accounts/edit.html"><img src="/images/modules/accounts/usredit.png" alt="Редактирование" /></a>
-->
<a href="/accounts/exit.html">{$lang.exit}</a>
{else}
<br />{$lang.hello}{$lang.guest}!<br />{$lang.notautorize}<br /> <a href="/accounts/authorization.html" onclick="/*window_open('autorize');*/">{$lang.enter}</a>
{/if}
</div>
</div>
{if $_SESSION.user->PHPSESSID ne ''}
<div id="menu">
{if $_SESSION.user->access.main.index eq on}<a href="/main/info.html">{$lang.main}</a>{/if}
{if $_SESSION.user->access.bots.index eq on}<a href="/bots/">{$lang.bots}</a>{/if}
{if $_SESSION.user->access.bots.jobs eq on}<a href="/bots/jobs.html">{$lang.jobs}</a>{/if}
{if $_SESSION.user->access.logs.index eq on}<a href="/logs/index.html">{$lang.logs}</a>{/if}
{if $_SESSION.user->access.filters.index eq on}<a href="/filters/">{$lang.filters}</a>{/if}
{if $_SESSION.user->access.cabs.index eq on}<a href="/cabs/index.html">{$lang.cabs}</a>{/if}
{if $_SESSION.user->access.keylog.index eq on}<a href="/keylog/index.html">{$lang.keylog}</a>{/if}
{if $_SESSION.user->access.autosys.index eq on}<a href="/autosys/domains.html">{$lang.autosys}</a>{/if}
{if $_SESSION.user->access.settings.index eq on}<a href="/settings/">{$lang.settings}</a>{/if}
{if $_SESSION.user->access.accounts.index eq on}<a href="/accounts/">{$lang.accounts}</a>{/if}
<div id="menu_account"></div>
</div>
<div id="position"></div>
{/if}
<div id="content">{include file="$site_data"}</div>
<br />

<div id="footer"><div class="left"></div><div class="right">© 2009-{$smarty.now|date_format:"%Y"}</div></div>
<br /><br />
</div>

</body>
</html>
