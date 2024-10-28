import { bot } from "./bot";
import { server } from "./wss";

bot.launch(() => {
  console.log("BOT started");
});

server.listen(Number(process.env.WSS_PORT), () => {
  console.log("WS Server started");
});
