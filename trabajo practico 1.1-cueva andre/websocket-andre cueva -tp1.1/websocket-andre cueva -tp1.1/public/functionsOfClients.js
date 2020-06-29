//funcion para saber el tiempo con hora y minuto.
function timeDay(){
	var day= new Date();
	var minute= day.getMinutes();
	var hour= day.getHours();
	var retorno = hour+":"+minute;
	return retorno;
}
function EventCloseButton(){//evitara que reaccione a una accion.
	buttonOfUser.style.pointerEvents = "none";
	buttonOfAdmin.style.pointerEvents = "none";
	nameOfClient.style.pointerEvents="none";
	buttonUser.style.display="none";
}
function ButtonOfUserAndAdminDisabled(buttonOfUser,buttonOfAdmin){//deshabilito los bottones
	buttonOfUser.disabled = true;//deshabilito el boton user
	buttonOfAdmin.disabled = true;//deshabilito el boton admin
	EventCloseButton();
}   

var socket = io.connect('http://localhost:4000');

var message= document.getElementById('Primero');

var messageOfClient = document.getElementById('Segundo');
var nameOfClient = document.getElementById('nameOfClient');

var buttonOfUser = document.getElementById('user');
var buttonOfAdmin = document.getElementById('admin');


//----------------EVENTOS----------------------------------
buttonOfUser.addEventListener('click', function(){//funcion click de buttonUser	
	socket.emit('clickOfUser',nameOfClient.value );

	messageOfClient.style.display="block";//El elemento se representa como un elemento de nivel de bloque
	messageOfClient.innerHTML= '<p>AHORA SOS USUARIO:'+nameOfClient.value+'</p>';//copio este <p></p> en messageOfClient
	
	ButtonOfUserAndAdminDisabled(buttonOfUser,buttonOfAdmin);
	
});

buttonOfAdmin.addEventListener('click', function(){//funcion click de buttonAdmin
    socket.emit('clickOfAdmin');

    messageOfClient.style.display="block";
    messageOfClient.innerHTML = '<p>AHORA SOS ADMINISTRADOR</p>';

    ButtonOfUserAndAdminDisabled(buttonOfUser,buttonOfAdmin);

});

//---------------------------------------------------------

socket.on('clickOfAdmin', function(){
    message.style.display="block";
});

socket.on('clickOfUser', function(Name,idOfSocket){ 
	
    message.innerHTML += '<p>__________________________________________________________<br>NameOfNewClient:'+Name+ '<br><br>idOfSocket: '+idOfSocket+'.<br><br>Time of Client: '+timeDay()+'</p><span id="IdSpan">estado</span>';
    document.getElementById("IdSpan").id = idOfSocket;
    document.getElementById(idOfSocket).style.color="#7FFF00";

});
//----------------------------------------------------------

socket.on('disconnectOfUser', function(idOfSocket){
    document.getElementById(idOfSocket).style.color="#DC143C";
});
//----------------------------------------------------------