/*������� �������, ��������� �� ����� -> ������� -> ������� ������� */
var interval;

$(document).ready(function(){
	/*������� �������� ���������*/
	interval = setInterval("disableButton()", 100);
});

/*���������� ������ ���������� �� "�������"*/
function disableButton()
{
	/*���� � ������*/
	var btnPath = "table#EDITFORM tr td button";
	
	if($(btnPath) != undefined)
	{
		$(btnPath).attr("disabled", true);
		
		/*��� ������ 9 ������� �������*/
		$("#ScrollHeader").html("������� ������� � ������������� �������, <font color='red'>�������� ��������!</font>");
		
		/*��������� ��������*/
		clearInterval(interval);
	}
}