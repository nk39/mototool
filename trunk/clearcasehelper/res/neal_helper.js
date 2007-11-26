function perform_login(name, pass)
{
	cipt_login_form.cid.value=name;
	cipt_login_form.pswd.value=pass;
	
	cipt_login_form.submit.click();
}

function sayHello()
{
	alert('hi');
}

function perform_compare(fold, fnew)
{
  alert("old file: " + fold);
  alert("new file: " + fnew);
  
  twoFile.oldfile.value = fold;
  twoFile.newfile.value = fnew;
  
  alert("old file: " + twoFile.oldfile.value);
  alert("new file: " + twoFile.newfile.value);
  twoFile.submit.click();
}
