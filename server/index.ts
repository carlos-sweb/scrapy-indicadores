import {serve} from "bun"
serve({
	port:9000,
	routes:{
		"/": async req =>{			
			const indicadores = await req.json()
			console.log(indicadores)
			return new Response("Ok")
		}
	}
})