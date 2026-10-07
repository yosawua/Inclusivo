// Fetch API para consultar el estado del hardware asíncronamente
setInterval(() => {
    // Al documentar en GitHub, esta URL asume que el ESP32 emite el JSON en esta ruta
    fetch('/api/estado')
    .then(response => response.json())
    .then(data => {
        // Mapeo del estado de los 8 limit switches
        for (let i = 0; i < 8; i++) {
            let elemento = document.getElementById('s' + i);
            if (data.sensores[i] === 1) {
                elemento.classList.add('active'); // Enciende el neón
            } else {
                elemento.classList.remove('active'); // Apaga el neón
            }
        }
        
        // Actualización del log del módulo DFPlayer Mini
        if (data.pista > 0) {
            document.getElementById('track-info').innerText = "Reproduciendo pista MP3: 00" + data.pista;
        }
    })
    .catch(error => console.error('Error de conexión con el microcontrolador:', error));
}, 300); // Polling cada 300ms para sensación de tiempo real