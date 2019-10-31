{if isset($parse_result) && $parse_result}
	<h4 class="success_header">
		����� ������� ���������!
	</h4>
	������������� ����� �������: <b>{$parse_new_ftp}</b>
	<br /><br />
{else if isset($parse_result) && !$parse_result}
	<h4 class="achtung_header">
	��������� ������ ��� ��������� ������!
	</h4><br />
{/if}

<table id="table_view_report_data" width="800" cellspacing="0">
	<tr><th width="100%">��������� � ������ �������</th></tr>
	{foreach from=$log_list item=log_item}
		<tr><td>
		{if strstr($log_item.log_line, 'ERR_')}
			<font style="font-weight: bold; color: #b72525">
			{$log_item.log_line}
		</font>
		{else}
			{$log_item.log_line}
		{/if}
		<font style="color: #bbbbbb; font-size: 10px;"> | {$log_item.log_extra}</font></td></tr>
	{/foreach}
</table>
