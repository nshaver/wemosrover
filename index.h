const char *html1="<html>\n\
	<head>\n\
		<meta charset=\"utf-8\">\n\
		<meta name=\"viewport\" content=\"width=device-width, user-scalable=no, minimum-scale=1.0, maximum-scale=1.0\">\n\
		\n\
		<style>\n\
		body {\n\
			overflow	: hidden;\n\
			padding		: 0;\n\
			margin		: 0;\n\
			background-color: #BBB;\n\
		}\n\
		#info {\n\
			position	: absolute;\n\
			top		: 0px;\n\
			width		: 100%;\n\
			padding		: 5px;\n\
			text-align	: center;\n\
		}\n\
		#info a {\n\
			color		: #66F;\n\
			text-decoration	: none;\n\
		}\n\
		#info a:hover {\n\
			text-decoration	: underline;\n\
		}\n\
		#container {\n\
			left			: 0%;\n\
			width			: 100%;\n\
			height		: 100%;\n\
			overflow	: hidden;\n\
			padding		: 0;\n\
			margin		: 0;\n\
			border-style: solid;\n\
			-webkit-user-select	: none;\n\
			-moz-user-select	: none;\n\
		}\n\
		</style>\n\
	</head>\n\
	<body>\n\
		<div id=\"container\"></div>\n\
		<div id=\"info\">\n\
			<span id=\"result\"></span>\n\
		</div> \n\
		<script>\n\
	var VirtualJoystick	= function(opts) {\n\
		opts			= opts			|| {};\n\
		this._container		= opts.container	|| document.body;\n\
		this._strokeStyle	= opts.strokeStyle	|| 'cyan';\n\
		this._stickEl		= opts.stickElement	|| this._buildJoystickStick();\n\
		this._baseEl		= opts.baseElement	|| this._buildJoystickBase();\n\
		this._mouseSupport	= opts.mouseSupport !== undefined ? opts.mouseSupport : false;\n\
		this._stationaryBase	= opts.stationaryBase || false;\n\
		this._baseX		= this._stickX = opts.baseX || 0\n\
		this._baseY		= this._stickY = opts.baseY || 0\n\
		this._limitStickTravel	= opts.limitStickTravel || false\n\
		this._stickRadius	= opts.stickRadius !== undefined ? opts.stickRadius : 100\n\
		this._useCssTransform	= opts.useCssTransform !== undefined ? opts.useCssTransform : false\n\
		this._container.style.position	= \"relative\"\n\
		this._container.appendChild(this._baseEl)\n\
		this._baseEl.style.position	= \"absolute\"\n\
		this._baseEl.style.display	= \"none\"\n\
		this._container.appendChild(this._stickEl)\n\
		this._stickEl.style.position	= \"absolute\"\n\
		this._stickEl.style.display	= \"none\"\n\
		this._pressed	= false;\n\
		this._touchIdx	= null;\n\
		\n\
		if(this._stationaryBase === true){\n\
			this._baseEl.style.display	= \"\";\n\
			this._baseEl.style.left		= (this._baseX - this._baseEl.width /2)+\"px\";\n\
			this._baseEl.style.top		= (this._baseY - this._baseEl.height/2)+\"px\";\n\
		}\n\
			\n\
		this._transform	= this._useCssTransform ? this._getTransformProperty() : false;\n\
		this._has3d	= this._check3D();\n\
		\n\
		var __bind	= function(fn, me){ return function(){ return fn.apply(me, arguments); }; };\n\
		this._$onTouchStart	= __bind(this._onTouchStart	, this);\n\
		this._$onTouchEnd	= __bind(this._onTouchEnd	, this);\n\
		this._$onTouchMove	= __bind(this._onTouchMove	, this);\n\
		this._container.addEventListener( 'touchstart'	, this._$onTouchStart	, false );\n\
		this._container.addEventListener( 'touchend'	, this._$onTouchEnd	, false );\n\
		this._container.addEventListener( 'touchmove'	, this._$onTouchMove	, false );\n\
		if( this._mouseSupport ){\n\
			this._$onMouseDown	= __bind(this._onMouseDown	, this);\n\
			this._$onMouseUp	= __bind(this._onMouseUp	, this);\n\
			this._$onMouseMove	= __bind(this._onMouseMove	, this);\n\
			this._container.addEventListener( 'mousedown'	, this._$onMouseDown	, false );\n\
			this._container.addEventListener( 'mouseup'	, this._$onMouseUp	, false );\n\
			this._container.addEventListener( 'mousemove'	, this._$onMouseMove	, false );\n\
		}\n\
	}\n\
	VirtualJoystick.prototype.destroy	= function() {\n\
		this._container.removeChild(this._baseEl);\n\
		this._container.removeChild(this._stickEl);\n\
		this._container.removeEventListener( 'touchstart'	, this._$onTouchStart	, false );\n\
		this._container.removeEventListener( 'touchend'		, this._$onTouchEnd	, false );\n\
		this._container.removeEventListener( 'touchmove'	, this._$onTouchMove	, false );\n\
		if( this._mouseSupport ){\n\
			this._container.removeEventListener( 'mouseup'		, this._$onMouseUp	, false );\n\
			this._container.removeEventListener( 'mousedown'	, this._$onMouseDown	, false );\n\
			this._container.removeEventListener( 'mousemove'	, this._$onMouseMove	, false );\n\
		}\n\
	}\n\
	VirtualJoystick.touchScreenAvailable	= function() {\n\
		return 'createTouch' in document ? true : false;\n\
	}\n\
	;(function(destObj){\n\
		destObj.addEventListener	= function(event, fct){\n\
			if(this._events === undefined) 	this._events	= {};\n\
			this._events[event] = this._events[event]	|| [];\n\
			this._events[event].push(fct);\n\
			return fct;\n\
		};\n\
		destObj.removeEventListener	= function(event, fct){\n\
			if(this._events === undefined) 	this._events	= {};\n\
			if( event in this._events === false  )	return;\n\
			this._events[event].splice(this._events[event].indexOf(fct), 1);\n\
		};\n\
		destObj.dispatchEvent		= function(event /* , args... */){\n\
			if(this._events === undefined) 	this._events	= {};\n\
			if( this._events[event] === undefined )	return;\n\
			var tmpArray	= this._events[event].slice(); \n\
			for(var i = 0; i < tmpArray.length; i++){\n\
				var result	= tmpArray[i].apply(this, Array.prototype.slice.call(arguments, 1))\n\
				if( result !== undefined )	return result;\n\
			}\n\
			return undefined\n\
		};\n\
	})(VirtualJoystick.prototype);\n\
	VirtualJoystick.prototype.deltaX	= function(){ return this._stickX - this._baseX;	}\n\
	VirtualJoystick.prototype.deltaY	= function(){ return this._stickY - this._baseY;	}\n\
	VirtualJoystick.prototype.up	= function(){\n\
		if( this._pressed === false )	return false;\n\
		var deltaX	= this.deltaX();\n\
		var deltaY	= this.deltaY();\n\
		if( deltaY >= 0 )				return false;\n\
		if( Math.abs(deltaX) > 2*Math.abs(deltaY) )	return false;\n\
		return true;\n\
	}\n\
	VirtualJoystick.prototype.down	= function(){\n\
		if( this._pressed === false )	return false;\n\
		var deltaX	= this.deltaX();\n\
		var deltaY	= this.deltaY();\n\
		if( deltaY <= 0 )				return false;\n\
		if( Math.abs(deltaX) > 2*Math.abs(deltaY) )	return false;\n\
		return true;	\n\
	}\n\
	VirtualJoystick.prototype.right	= function(){\n\
		if( this._pressed === false )	return false;\n\
		var deltaX	= this.deltaX();\n\
		var deltaY	= this.deltaY();\n\
		if( deltaX <= 0 )				return false;\n\
		if( Math.abs(deltaY) > 2*Math.abs(deltaX) )	return false;\n\
		return true;	\n\
	}\n\
	VirtualJoystick.prototype.left	= function(){\n\
		if( this._pressed === false )	return false;\n\
		var deltaX	= this.deltaX();\n\
		var deltaY	= this.deltaY();\n\
		if( deltaX >= 0 )				return false;\n\
		if( Math.abs(deltaY) > 2*Math.abs(deltaX) )	return false;\n\
		return true;	\n\
	}\n\
	VirtualJoystick.prototype._onUp	= function() {\n\
		this._pressed	= false; \n\
		this._stickEl.style.display	= \"none\";\n\
		\n\
		if(this._stationaryBase == false){	\n\
			this._baseEl.style.display	= \"none\";\n\
		\n\
			this._baseX	= this._baseY	= 0;\n\
			this._stickX	= this._stickY	= 0;\n\
		}\n\
	}\n\
	VirtualJoystick.prototype._onDown	= function(x, y) {\n\
		this._pressed	= true; \n\
		if(this._stationaryBase == false){\n\
			this._baseX	= x;\n\
			this._baseY	= y;\n\
			this._baseEl.style.display	= \"\";\n\
			this._move(this._baseEl.style, (this._baseX - this._baseEl.width /2), (this._baseY - this._baseEl.height/2));\n\
		}\n\
		\n\
		this._stickX	= x;\n\
		this._stickY	= y;\n\
		\n\
		if(this._limitStickTravel === true){\n\
			var deltaX	= this.deltaX();\n\
			var deltaY	= this.deltaY();\n\
			var stickDistance = Math.sqrt( (deltaX * deltaX) + (deltaY * deltaY) );\n\
			if(stickDistance > this._stickRadius){\n\
				var stickNormalizedX = deltaX / stickDistance;\n\
				var stickNormalizedY = deltaY / stickDistance;\n\
				\n\
				this._stickX = stickNormalizedX * this._stickRadius + this._baseX;\n\
				this._stickY = stickNormalizedY * this._stickRadius + this._baseY;\n\
			} 	\n\
		}\n\
		\n\
		this._stickEl.style.display	= \"\";\n\
		this._move(this._stickEl.style, (this._stickX - this._stickEl.width /2), (this._stickY - this._stickEl.height/2));	\n\
	}\n\
	VirtualJoystick.prototype._onMove	= function(x, y) {\n\
		if( this._pressed === true ){\n\
			this._stickX	= x;\n\
			this._stickY	= y;\n\
			\n\
			if(this._limitStickTravel === true){\n\
				var deltaX	= this.deltaX();\n\
				var deltaY	= this.deltaY();\n\
				var stickDistance = Math.sqrt( (deltaX * deltaX) + (deltaY * deltaY) );\n\
				if(stickDistance > this._stickRadius){\n\
					var stickNormalizedX = deltaX / stickDistance;\n\
					var stickNormalizedY = deltaY / stickDistance;\n\
				\n\
					this._stickX = stickNormalizedX * this._stickRadius + this._baseX;\n\
					this._stickY = stickNormalizedY * this._stickRadius + this._baseY;\n\
				} 		\n\
			}\n\
			\n\
						this._move(this._stickEl.style, (this._stickX - this._stickEl.width /2), (this._stickY - this._stickEl.height/2));	\n\
		}	\n\
	}\n\
	VirtualJoystick.prototype._onMouseUp	= function(event) {\n\
		return this._onUp();\n\
	}\n\
	VirtualJoystick.prototype._onMouseDown	= function(event) {\n\
		event.preventDefault();\n\
		var x	= event.clientX;\n\
		var y	= event.clientY;\n\
		return this._onDown(x, y);\n\
	}\n\
	VirtualJoystick.prototype._onMouseMove	= function(event) {\n\
		var x	= event.clientX;\n\
		var y	= event.clientY;\n\
		return this._onMove(x, y);\n\
	}\n\
	VirtualJoystick.prototype._onTouchStart	= function(event) {\n\
		// if there is already a touch inprogress do nothing\n\
		if( this._touchIdx !== null )	return;\n\
		// notify event for validation\n\
		var isValid	= this.dispatchEvent('touchStartValidation', event);\n\
		if( isValid === false )	return;\n\
		\n\
		// dispatch touchStart\n\
		this.dispatchEvent('touchStart', event);\n\
		event.preventDefault();\n\
		// get the first who changed\n\
		var touch	= event.changedTouches[0];\n\
		// set the touchIdx of this joystick\n\
		this._touchIdx	= touch.identifier;\n\
		// forward the action\n\
		var x		= touch.pageX;\n\
		var y		= touch.pageY;\n\
		return this._onDown(x, y)\n\
	}\n\
	VirtualJoystick.prototype._onTouchEnd	= function(event) {\n\
		// if there is no touch in progress, do nothing\n\
		if( this._touchIdx === null )	return;\n\
		// dispatch touchEnd\n\
		this.dispatchEvent('touchEnd', event);\n\
		// try to find our touch event\n\
		var touchList	= event.changedTouches;\n\
		for(var i = 0; i < touchList.length && touchList[i].identifier !== this._touchIdx; i++);\n\
		// if touch event isnt found, \n\
		if( i === touchList.length)	return;\n\
		// reset touchIdx - mark it as no-touch-in-progress\n\
		this._touchIdx	= null;\n\
	//??????\n\
	// no preventDefault to get click event on ios\n\
	event.preventDefault();\n\
		return this._onUp()\n\
	}\n\
	VirtualJoystick.prototype._onTouchMove	= function(event) {\n\
		// if there is no touch in progress, do nothing\n\
		if( this._touchIdx === null )	return;\n\
		// try to find our touch event\n\
		var touchList	= event.changedTouches;\n\
		for(var i = 0; i < touchList.length && touchList[i].identifier !== this._touchIdx; i++ );\n\
		// if touch event with the proper identifier isnt found, do nothing\n\
		if( i === touchList.length)	return;\n\
		var touch	= touchList[i];\n\
		event.preventDefault();\n\
		var x		= touch.pageX;\n\
		var y		= touch.pageY;\n\
		return this._onMove(x, y)\n\
	}\n\
	VirtualJoystick.prototype._buildJoystickBase	= function() {\n\
		var canvas	= document.createElement( 'canvas' );\n\
		canvas.width	= 126;\n\
		canvas.height	= 126;\n\
		\n\
		var ctx		= canvas.getContext('2d');\n\
		ctx.beginPath(); \n\
		ctx.strokeStyle = this._strokeStyle; \n\
		ctx.lineWidth	= 6; \n\
		ctx.arc( canvas.width/2, canvas.width/2, 40, 0, Math.PI*2, true); \n\
		ctx.stroke();	\n\
		ctx.beginPath(); \n\
		ctx.strokeStyle	= this._strokeStyle; \n\
		ctx.lineWidth	= 2; \n\
		ctx.arc( canvas.width/2, canvas.width/2, 60, 0, Math.PI*2, true); \n\
		ctx.stroke();\n\
		\n\
		return canvas;\n\
	}\n\
	VirtualJoystick.prototype._buildJoystickStick	= function() {\n\
		var canvas	= document.createElement( 'canvas' );\n\
		canvas.width	= 86;\n\
		canvas.height	= 86;\n\
		var ctx		= canvas.getContext('2d');\n\
		ctx.beginPath(); \n\
		ctx.strokeStyle	= this._strokeStyle; \n\
		ctx.lineWidth	= 6; \n\
		ctx.arc( canvas.width/2, canvas.width/2, 40, 0, Math.PI*2, true); \n\
		ctx.stroke();\n\
		return canvas;\n\
	}\n\
	VirtualJoystick.prototype._move = function(style, x, y) {\n\
		if (this._transform) {\n\
			if (this._has3d) {\n\
				style[this._transform] = 'translate3d(' + x + 'px,' + y + 'px, 0)';\n\
			} else {\n\
				style[this._transform] = 'translate(' + x + 'px,' + y + 'px)';\n\
			}\n\
		} else {\n\
			style.left = x + 'px';\n\
			style.top = y + 'px';\n\
		}\n\
	}\n\
	VirtualJoystick.prototype._getTransformProperty = function() {\n\
		var styles = [\n\
			'webkitTransform',\n\
			'MozTransform',\n\
			'msTransform',\n\
			'OTransform',\n\
			'transform'\n\
		];\n\
		var el = document.createElement('p');\n\
		var style;\n\
		for (var i = 0; i < styles.length; i++) {\n\
			style = styles[i];\n\
			if (null != el.style[style]) {\n\
				return style;\n\
			}\n\
		}         \n\
	}\n\
		\n\
	VirtualJoystick.prototype._check3D = function() {        \n\
		var prop = this._getTransformProperty();\n\
		// IE8<= doesn't have `getComputedStyle`\n\
		if (!prop || !window.getComputedStyle) return module.exports = false;\n\
		var map = {\n\
			webkitTransform: '-webkit-transform',\n\
			OTransform: '-o-transform',\n\
			msTransform: '-ms-transform',\n\
			MozTransform: '-moz-transform',\n\
			transform: 'transform'\n\
		};\n\
		// from: https://gist.github.com/lorenzopolidori/3794226\n\
		var el = document.createElement('div');\n\
		el.style[prop] = 'translate3d(1px,1px,1px)';\n\
		document.body.insertBefore(el, null);\n\
		var val = getComputedStyle(el).getPropertyValue(map[prop]);\n\
		document.body.removeChild(el);\n\
		var exports = null != val && val.length && 'none' != val;\n\
		return exports;\n\
	}\n\
	var thisurl=\"\";\n\
	var lasturl=\"\";\n\
	var thisaction=\"\";\n\
	var thisdist=0;\n\
	var reqid=0;\n\
	var waiting_on_reqid=0;\n\
	var last_reqid_ms=0;\n\
	var reqid_ms=0;\n\
	var curdt;\n\
	console.log(\"touchscreen is\", VirtualJoystick.touchScreenAvailable() ? \"available\" : \"not available\");\n\
	var joystick	= new VirtualJoystick({\n\
		container	: document.getElementById('container'),\n\
		mouseSupport	: true,\n\
	});\n\
	joystick.addEventListener('touchStart', function(){\n\
		console.log('down')\n\
	})\n\
	joystick.addEventListener('touchEnd', function(){\n\
		console.log('up')\n\
	})\n\
	setInterval(function(){\n\
		var outputEl	= document.getElementById('result');\n\
		thisaction='';\n\
		if (joystick.up()){\n\
			thisaction+='n';\n\
		} else if (joystick.down()){\n\
			thisaction+='s';\n\
		}\n\
		if (joystick.left()){\n\
			thisaction+='w';\n\
		} else if (joystick.right()){\n\
			thisaction+='e';\n\
		}\n\
		if (thisaction=='') thisaction='x';\n\
		thisdist=parseInt(Math.sqrt(Math.pow(joystick.deltaX(),2) + Math.pow(joystick.deltaY(),2)));\n\
		if (thisdist>200) thisdist=200;\n\
		thisdist=parseInt(thisdist/20);\n\
		if (thisdist==0) thisaction='x';\n\
		thisurl=\"/drive?drive=\"+thisaction+\"&dist=\"+thisdist;\n\
		outputEl.innerHTML	= '<b>Result:</b> '\n\
			+ ' dx:'+joystick.deltaX()\n\
			+ ' dy:'+joystick.deltaY()\n\
			+ ' '+thisurl;\n\
		if (thisurl != lasturl){\n\
			var xmlhttp;\n\
			if (window.XMLHttpRequest){\n\
				xmlhttp=new XMLHttpRequest();\n\
				xmlhttp.onreadystatechange=function(){\n\
					if (xmlhttp.readyState==4 && xmlhttp.status==200){\n\
						try {\n\
							var js=JSON.parse(xmlhttp.responseText);\n\
							if (js.reqid==waiting_on_reqid){\n\
								// done waiting\n\
								curdt=new Date();\n\
								reqid_ms=curdt.getTime()-last_reqid_ms;\n\
								console.log(\"reqid:\"+js.reqid+\" status:\"+js.status+\" time:\"+reqid_ms+\"ms\");\n\
								waiting_on_reqid=0;\n\
							}\n\
						} catch (e) {\n\
							// bad return\n\
							console.log(\"bad response: \"+xmlhttp.responseText);\n\
						}\n\
					}\n\
				}\n\
				if (waiting_on_reqid>0){\n\
					// still waiting on return\n\
					curdt=new Date();\n\
					if ((curdt.getTime()-last_reqid_ms)>1000){\n\
						// give up\n\
						console.log(\"!!! giving up on reqid \"+waiting_on_reqid+\", sending stop\");\n\
						reqid++;\n\
						thisurl=\"/drive?drive=x&dist=0\";\n\
						xmlhttp.open(\"GET\", thisurl+'&reqid='+reqid, true);\n\
						console.log(\"url: \"+thisurl+'&reqid='+reqid);\n\
						xmlhttp.send();\n\
						waiting_on_reqid=reqid;\n\
						curdt=new Date();\n\
						last_reqid_ms=curdt.getTime();\n\
						lasturl=thisurl;\n\
					}\n\
				} else {\n\
					reqid++;\n\
					xmlhttp.open(\"GET\", thisurl+'&reqid='+reqid, true);\n\
					console.log(\"url: \"+thisurl+'&reqid='+reqid);\n\
					xmlhttp.send();\n\
					waiting_on_reqid=reqid;\n\
					curdt=new Date();\n\
					last_reqid_ms=curdt.getTime();\n\
					lasturl=thisurl;\n\
				}\n\
			}\n\
		};\n\
	}, 250);\n\
</script>\n\
</body>\n\
</html>";
