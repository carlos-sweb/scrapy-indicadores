import htmlContent from "./index.html" with { type: "text" };

const server = Bun.serve({
  port: 3000,
  fetch(request) {    
    return new Response(htmlContent);
  },
});

console.log(`Listening on ${server.url}`);