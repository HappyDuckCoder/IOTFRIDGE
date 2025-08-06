import requests

bot_token = '8494411737:AAEzzS2OPxWpwsPQCQuI5HxopCYpFxg-ChU'
url = f'https://api.telegram.org/bot{bot_token}/sendMessage'
chat_id = '7755309376'
message_test = 'Test message'

class Notification:
    def __init__(): 
        pass

    @staticmethod 
    def sendMessage(text):
        payload = {'chat_id': chat_id, 'text': text}
        response = requests.post(url, data=payload)
        
        if response.json().get('ok'):
            print('Message sent successfully')
        else:
            print('Failed to send message_test')


if __name__ == '__main__':
    Notification.sendMessage(message_test)


