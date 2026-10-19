import React, { useEffect, useRef } from 'react';

export type ToastVariant = 'success' | 'info' | 'warning' | 'error';

export interface ToastNotification {
  id: string;
  variant: ToastVariant;
  title: string;
  description: string;
  duration?: number;
}

interface ToastProps {
  notification: ToastNotification | null;
  onDismiss: () => void;
}

export const Toast: React.FC<ToastProps> = ({ notification, onDismiss }) => {
  const timerRef = useRef<number | null>(null);

  useEffect(() => {
    if (!notification) return;

    const duration = notification.duration ?? (notification.variant === 'error' ? 8000 : 4500);

    // Set auto-dismiss timer
    timerRef.current = window.setTimeout(() => {
      onDismiss();
    }, duration);

    return () => {
      if (timerRef.current !== null) {
        window.clearTimeout(timerRef.current);
        timerRef.current = null;
      }
    };
  }, [notification, onDismiss]);

  const handleMouseEnter = () => {
    if (timerRef.current !== null) {
      window.clearTimeout(timerRef.current);
      timerRef.current = null;
    }
  };

  const handleMouseLeave = () => {
    if (!notification) return;
    const duration = notification.duration ?? (notification.variant === 'error' ? 8000 : 4500);
    timerRef.current = window.setTimeout(() => {
      onDismiss();
    }, duration / 2); // remaining grace period
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Escape') {
      onDismiss();
    }
  };

  if (!notification) return null;

  const isAlert = notification.variant === 'error';

  return (
    <div
      className="toast-container"
      role={isAlert ? 'alert' : 'status'}
      aria-live={isAlert ? 'assertive' : 'polite'}
      aria-atomic="true"
      onKeyDown={handleKeyDown}
      onMouseEnter={handleMouseEnter}
      onMouseLeave={handleMouseLeave}
    >
      <div className={`toast-card toast-${notification.variant}`}>
        {/* Status Icon */}
        <div className="toast-icon-wrapper" aria-hidden="true">
          {notification.variant === 'success' && (
            <svg className="toast-svg-icon" viewBox="0 0 20 20" fill="currentColor">
              <path
                fillRule="evenodd"
                d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-9.293a1 1 0 00-1.414-1.414L9 10.586 7.707 9.293a1 1 0 00-1.414 1.414l2 2a1 1 0 001.414 0l4-4z"
                clipRule="evenodd"
              />
            </svg>
          )}
          {notification.variant === 'info' && (
            <svg className="toast-svg-icon" viewBox="0 0 20 20" fill="currentColor">
              <path
                fillRule="evenodd"
                d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-7-4a1 1 0 11-2 0 1 1 0 012 0zM9 9a1 1 0 000 2v3a1 1 0 001 1h1a1 1 0 100-2v-3a1 1 0 00-1-1H9z"
                clipRule="evenodd"
              />
            </svg>
          )}
          {notification.variant === 'warning' && (
            <svg className="toast-svg-icon" viewBox="0 0 20 20" fill="currentColor">
              <path
                fillRule="evenodd"
                d="M8.257 3.099c.765-1.36 2.722-1.36 3.486 0l5.58 9.92c.75 1.334-.213 2.98-1.742 2.98H4.42c-1.53 0-2.493-1.646-1.743-2.98l5.58-9.92zM11 13a1 1 0 11-2 0 1 1 0 012 0zm-1-8a1 1 0 00-1 1v3a1 1 0 002 0V6a1 1 0 00-1-1z"
                clipRule="evenodd"
              />
            </svg>
          )}
          {notification.variant === 'error' && (
            <svg className="toast-svg-icon" viewBox="0 0 20 20" fill="currentColor">
              <path
                fillRule="evenodd"
                d="M10 18a8 8 0 100-16 8 8 0 000 16zM8.707 7.293a1 1 0 00-1.414 1.414L8.586 10l-1.293 1.293a1 1 0 101.414 1.414L10 11.414l1.293 1.293a1 1 0 001.414-1.414L11.414 10l1.293-1.293a1 1 0 00-1.414-1.414L10 8.586 8.707 7.293z"
                clipRule="evenodd"
              />
            </svg>
          )}
        </div>

        {/* Text Content */}
        <div className="toast-content">
          <div className="toast-title">{notification.title}</div>
          <div className="toast-description">{notification.description}</div>
        </div>

        {/* Dismiss Button */}
        <button
          type="button"
          className="toast-dismiss-btn"
          onClick={onDismiss}
          title="Dismiss notification (Esc)"
          aria-label="Dismiss notification"
        >
          ✕
        </button>
      </div>
    </div>
  );
};
