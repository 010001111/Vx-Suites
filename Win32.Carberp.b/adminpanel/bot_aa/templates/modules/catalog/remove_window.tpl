{if $save eq true}
<form action="#" method="post" name="remove{$rand_name}" id="remove{$rand_name}">
<input type="hidden" />
</form>
<script language="javascript" type="application/javascript">
window_close_opacity(document.forms['remove{$rand_name}'].parentNode.parentNode.id, 1);
{literal}
hax('/catalog/?ajax=1',{id: 'cats_content',nohistory:true,nocache:true,destroy:true,onload: function (){$("#cats").treeview({animated: "fast",collapsed: true,persist: "cookie",cookieId: "logs-treeview-edit"});},rc:true})
{/literal}
</script>
{else}
<div align="center" style="text-align:center; width: 750px;">
{if $parent->host eq ''}
<h2><span style="color: #F00">Внимание!</span><br />Вы хотите удалить раздел!</h2>
<br />
<div style="font-size: 16px;">
<div>Информация о фильтрах, в фильтрах, о разделах, в разделах будет полностью утрачена!</div><br />
<div>При удаление раздела удаляться все внутренние разделы и фильтры!</div><br />
<div>Если в разделе и во всех подразделах есть рабочие фильтры,<br />то они удаляться и все данные фильтров этих тоже удаляться.</div><br />
<div>После удаления раздела, невозможно будет восстановить данные фильтров!</div><br />
</div>

<h2>Точно удалить раздел?</h2>
(<span style="font-size:14px">{$parent->name}</span>)
<br /><br />
<form action="/catalog/remove-{$parent->id}.html?window=1" method="post" name="remove{$rand_name}" id="remove{$rand_name}">
<input type="button" name="yes" value="Да, удалить!" onclick="submit_{$rand_name}();"/>
<input type="button" name="no" value="Нет, не удалять!" onclick="nosubmit_{$rand_name}();" />
</form>
{else}
<h2><span style="color: #F00">Внимание!</span><br />Вы хотите удалить фильтр!</h2>
<br />
<div style="font-size: 16px;">
<div>Сам фильтр и вся информация будет удалена без возможности востановления!</div><br />
</div>

<h2>Точно удалить фильтр?</h2>
(<span style="font-size:14px">{$parent->name}</span>)
<br /><br />
<form action="/catalog/remove-{$parent->id}.html?window=1" method="post" name="remove{$rand_name}" id="remove{$rand_name}">
<input type="button" name="yes" value="Да, удалить!" onclick="submit_{$rand_name}();"/>
<input type="button" name="no" value="Нет, не удалять!" onclick="nosubmit_{$rand_name}();" />
</form>
{/if}
</div>
<script language="javascript" type="application/javascript">
var id_{$rand_name} = document.forms['remove{$rand_name}'].parentNode.parentNode.id.replace('_content', '');
document.getElementById(id_{$rand_name} + '_title').innerHTML = '{if $parent->host ne ''}Удаление фильтра{else}Удаление раздела{/if}';
function submit_{$rand_name}(){ldelim}
hax(document.forms['remove{$rand_name}'].action,
	{ldelim}
		method: document.forms['remove{$rand_name}'].method,
		form: 'remove{$rand_name}',
		id: id_{$rand_name} + '_content',
		nohistory:true,
		nocache:true,
		destroy:true,
		rc:true
	{rdelim}
)
{rdelim};
function nosubmit_{$rand_name}(){ldelim}
window_close_opacity(document.forms['remove{$rand_name}'].parentNode.parentNode.parentNode.id, 1);
{rdelim};
</script>
{/if}