package com.azheimer.service;

import com.azheimer.entity.Alert;
import com.azheimer.entity.Caregiver;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

/**
 * Stub notification service.
 * Replace the log calls with real integrations:
 *  - Push: Firebase Admin SDK (FCM)
 *  - SMS:  Twilio REST client
 *  - Email: SendGrid / JavaMailSender
 */
@Slf4j
@Service
public class NotificationService {

    public void notify(Caregiver caregiver, Alert alert) {
        log.info("NOTIFY caregiver={} ({}) — [{}][{}] {}",
                caregiver.getId(), caregiver.getEmail(),
                alert.getSeverity(), alert.getAlertType(), alert.getMessage());

        sendPush(caregiver, alert);
        // sendSms(caregiver, alert);
        // sendEmail(caregiver, alert);
    }

    private void sendPush(Caregiver caregiver, Alert alert) {
        if (caregiver.getFcmToken() == null || caregiver.getFcmToken().isBlank()) {
            log.debug("No FCM token for caregiver {}, skipping push", caregiver.getId());
            return;
        }
        // TODO: integrate Firebase Admin SDK
        // Message message = Message.builder()
        //     .setToken(caregiver.getFcmToken())
        //     .setNotification(Notification.builder()
        //         .setTitle(alert.getAlertType().name())
        //         .setBody(alert.getMessage())
        //         .build())
        //     .build();
        // FirebaseMessaging.getInstance().send(message);
        log.info("Push sent (stub) to FCM token {} for alert {}", caregiver.getFcmToken(), alert.getId());
    }
}
