import { Telegraf, Markup, type Context } from "telegraf";
import { Flat, flatsRepo } from "./flats";
import { CacheClient } from "./cache";

type BotContext = Context & { flat?: Flat };

export const bot = new Telegraf<BotContext>(process.env.BOT_TOKEN);

const registerFlat = async (ctx: BotContext) => {
  const key = `register:${ctx.chat!.id}`;
  const registerStarted = Number(await CacheClient.get(key));
  if (!registerStarted) {
    await CacheClient.set(key, 1);
    await ctx.reply("Введите номер вашей квартиры. Он должен состоять и цифр");

    return;
  }

  if (ctx.text?.startsWith("/")) {
    await CacheClient.set(key, 0);

    return;
  }

  if (!/^\d+$/.test(ctx.text || "")) {
    await ctx.reply("Номер квартиры должен состоять из цифр");

    return;
  }

  const flatNumber = Number(ctx.text);

  await flatsRepo.upsert({ chatId: ctx.chat!.id, number: flatNumber });
  await CacheClient.set(key, 0);
  await ctx.reply("Квартира успешно привязана");
};

bot.use(async (ctx, next) => {
  const flat = await flatsRepo.getByChatId(ctx.chat!.id);
  if (!flat) {
    registerFlat(ctx);
    next();
  }
  ctx.flat = flat!;
  next();
});

bot.start((ctx) =>
  ctx.reply(
    "Выберите действие",
    Markup.inlineKeyboard([
      Markup.button.callback("Перепривязать квартиру", "change-flat"),
    ]),
  ),
);

bot.action("change-flat", registerFlat);