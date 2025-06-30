from twilio.rest import Client

# Your Twilio credentials
account_sid = '12c65c6af48fb8b89a3ace2200b17c35-07aa9abc-c509-4a75-918b-36f526c3ac64'
auth_token = 'your_auth_token'

client = Client(account_sid, auth_token)

message = client.messages.create(
    body="Xin chào, đây là tin nhắn từ Twilio!",
    from_='447491163443',    # Số điện thoại Twilio của bạn
    to='+84916821170'        # Số người nhận (format quốc tế, bắt đầu bằng +84 cho Việt Nam)
)

print("Message SID:", message.sid)


