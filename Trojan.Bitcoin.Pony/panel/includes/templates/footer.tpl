			</div>
	</div>

{if $authentication_success}
	<div class="push"></div>
</div>
   <div class="footer">
	<div id="foot">
    	<ul class="links">
{append var="nav_links" value="�������" index=""}
{append var="nav_links" value="FTP ������" index="ftp"}
{if $enable_http_mode && ($show_http_to_users || $priv_is_admin)}
{append var="nav_links" value="HTTP ������" index="http"}
{/if}
{if $show_other_to_users || $priv_is_admin}
{append var="nav_links" value="������" index="other"}
{append var="nav_links" value="����������" index="stats"}
{/if}
{if $show_domains && ($show_domains_to_users || $priv_is_admin)}
{append var="nav_links" value="������" index="ping"}
{/if}
{append var="nav_links" value="����" index="log"}
{append var="nav_links" value="������" index="reports"}
{append var="nav_links" value="����������" index="admin"}
{if $show_help_to_users || $priv_is_admin}
{append var="nav_links" value="������" index="help"}
{/if}
{append var="nav_links" value="�����" index="exit"}

{foreach from=$nav_links key=action item=contents name=nav}
			<li><a href="{$smarty.server.SCRIPT_NAME}?token={$token}{if $action != ""}&amp;action={$action}{/if}">{$contents}</a>{if !$smarty.foreach.nav.last} | {/if}</li>
{/foreach}
     	</ul>
    </div>
	</div>
{/if}

</body>
</html>