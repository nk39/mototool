function perform_login(var name, var password)
{
	cipt_login_form.cid=name;
	cipt_login_form.pswd=password;
	
	alert("name: " + cipt_login_form.cid);
	alert("pass: " + cipt_login_form.pswd);
	cipt_login_form.submit();
}
