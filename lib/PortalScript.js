// script used when Manager Portal is open (/param)
document.addEventListener(`DOMContentLoaded`, function() {
  if (window.location.pathname === `/param`) {
    try {
      var paramsKV = window.location.search.substring(1).split(`&`);
      if (!paramsKV || paramsKV.length < 1) return;
      paramsKV.forEach(function(paramKV) {
        console.log(`set param : `, paramKV);
        var paramKey = paramKV.split(`=`)[0];
        var paramValue = paramKV.split(`=`)[1];
        if (paramKey && document.getElementById(paramKey)) {
          document.getElementById(paramKey).value = paramValue;
        }
        return;
      });
    } catch (e) {
      console.log(`WARNING`, e);
    }
  }
});
