import { model, Schema } from 'mongoose';

export interface Flat {
    number: number;
    chatId: number;
}

const FlatSchema = new Schema<Flat>({
    number: { type: Number, index: true, required: true },
    chatId: { type: Number, index: true, unique: true, required: true },
});

const FlatModel = model<Flat>('flats', FlatSchema);

export class FlatsRepository {
    async getByChatId(chatId: number): Promise<Flat | null> {
        return FlatModel.findOne({ chatId }).lean();
    }

    async getManyByNumber(number: number): Promise<Flat[]> {
        return FlatModel.find({ number }).lean();
    }

    async update(
        chatId: number,
        updateData: Partial<Omit<Flat, 'chatId'>>
    ): Promise<Flat | null> {
        return FlatModel.findOneAndUpdate({ chatId }, updateData, {
            new: true,
        }).lean();
    }

    async upsert(flat: Flat): Promise<Flat> {
        return FlatModel.findOneAndUpdate({ chatId: flat.chatId }, flat, {
            new: true,
            upsert: true,
        }).lean();
    }
}

export const flatsRepo = new FlatsRepository();
