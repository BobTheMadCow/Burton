//  file: pebble-js-app.js
//  auth: Matthew Clark, SetPebble

// change this token for your project
var setPebbleToken = '3PWE';

Pebble.addEventListener('ready', function(e) {
//	console.log('JS - PebbleKit JS is ready.');
});

Pebble.addEventListener('appmessage', function(e) 
{
//	console.log('JS - appmessage eventListener triggered');
	key = e.payload.action;
//	console.log('JS - Key = %d', key);
	if (typeof(key) != 'undefined') 
	{
		var settings = localStorage.getItem(setPebbleToken);
		if (typeof(settings) == 'string') 
		{
			try 
			{
//				console.log('JS - Sending message');
				Pebble.sendAppMessage(JSON.parse(settings));
			}
			catch (e) 
			{
				console.log('JS - Error: ' + e);
			}
		}
		var request = new XMLHttpRequest();
//		console.log('JS - Openning http request as %s', Pebble.getAccountToken());
		request.open('GET', 'http://x.SetPebble.com/api/' + setPebbleToken + '/' + Pebble.getAccountToken(), true);
		request.onload = function(e)
		{
//			console.log('JS - Request loaded. Status %d', request.readyState);
			if (request.readyState == 4)
			if (request.status == 200)
			try 
			{
//				console.log('JS - Sending message');
				Pebble.sendAppMessage(JSON.parse(request.responseText));
			} 
			catch (e) 
			{
				console.log('JS - Error: ' + e);
			}
		}
		request.send(null);
	}
});

Pebble.addEventListener('showConfiguration', function(e) 
{
//	console.log('JS - showConfiguration eventListener triggered!');
	Pebble.openURL('http://x.SetPebble.com/' + setPebbleToken + '/' + Pebble.getAccountToken());
});

Pebble.addEventListener('webviewclosed', function(e) 
{
//	console.log('JS - webviewclosed eventListener triggered!');
	if ((typeof(e.response) == 'string') && (e.response.length > 0)) 
	{
//		console.log('JS - response = ' + e.response);
		try 
		{
//			console.log('JS - Sending message');
			Pebble.sendAppMessage(JSON.parse(e.response));
//			console.log('JS - Setting item');
			localStorage.setItem(setPebbleToken, e.response);
		} 
		catch(e) 
		{
			console.log('JS - Error: ' + e);
		}
	}
});