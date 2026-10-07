require('dotenv').config();
const fs = require('fs');
const path = require('path');

// Asegúrate de tener instalados: npm install dotenv axios
const axios = require('axios');

const apiKey = process.env.ELEVENLABS_API_KEY;
// Puedes cambiar este ID por la voz en español que más te guste de tu cuenta
const voiceId = 'pNInz6obbf5AWCGq4A3f'; 

const figuras = [
    { pista: '001', texto: '¡Cuadrado!' },
    { pista: '002', texto: '¡Círculo!' },
    { pista: '003', texto: '¡Trapecio!' },
    { pista: '004', texto: '¡Hexágono!' },
    { pista: '005', texto: '¡Triángulo!' },
    { pista: '006', texto: '¡Rectángulo!' },
    { pista: '007', texto: '¡Rombo!' },
    { pista: '008', texto: '¡Pentágono!' }
];

// Creamos la carpeta de salida si no existe
const outputDir = path.join(__dirname, '../Assets/audio_sd');
if (!fs.existsSync(outputDir)) {
    fs.mkdirSync(outputDir, { recursive: true });
}

async function generarVoz(figura) {
    const url = `https://api.elevenlabs.io/v1/text-to-speech/${voiceId}`;
    
    const headers = {
        'Accept': 'audio/mpeg',
        'xi-api-key': apiKey,
        'Content-Type': 'application/json'
    };

    const data = {
        text: figura.texto,
        model_id: "eleven_multilingual_v2",
        voice_settings: {
            stability: 0.5,
            similarity_boost: 0.75
        }
    };

    try {
        console.log(`Generando audio para: ${figura.texto}...`);
        const response = await axios.post(url, data, { headers, responseType: 'stream' });
        
        const filePath = path.join(outputDir, `${figura.pista}.mp3`);
        const writer = fs.createWriteStream(filePath);
        
        response.data.pipe(writer);
        
        return new Promise((resolve, reject) => {
            writer.on('finish', () => {
                console.log(`✅ Guardado: ${figura.pista}.mp3`);
                resolve();
            });
            writer.on('error', reject);
        });
    } catch (error) {
        console.error(`❌ Error con la figura ${figura.texto}:`, error.message);
    }
}

async function ejecutar() {
    for (const fig of figuras) {
        await generarVoz(fig);
    }
    console.log("¡Todos los audios generados correctamente para el DFPlayer!");
}

ejecutar(); 