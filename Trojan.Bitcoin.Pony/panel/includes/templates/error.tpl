<h4 class="achtung_header">
{if $err_code == 'ERR_NOT_ENOUGH_PRIVILEGES'}
	��������� ������: �� ������� ����������!
{elseif $err_code == 'ERR_UNK_LOG_ID'}
	��������� ������: �������� ������������� ����!
{elseif $err_code == 'ERR_UNK_REPORT_ID'}
	��������� ������: �������� ������������� ������!
{elseif $err_code == 'ERR_SRV_CONFIGURATION'}
	��������! �������� � ������������� �������!
{elseif $err_code == 'ERR_WRONG_PASSWORD'}
	��������� ������: �������� ������!
{elseif $err_code == 'ERR_PASSWORD_MISMATCH'}
	��������� ������: ����� ������ � ������������� �� ���������!
{elseif $err_code == 'ERR_EMPTY_PASSWORD'}
	��������� ������: ������ ������!
{else}
	��������� ������!
{/if}
</h4><br />

{if isset($back_link)}
<a href="{$smarty.server.SCRIPT_NAME}{$back_link nofilter}">��������� � ����������� �����</a>.
<br /><br />
{/if}