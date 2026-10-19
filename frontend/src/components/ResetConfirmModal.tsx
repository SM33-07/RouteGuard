import React, { useEffect, useRef } from 'react';

interface ResetConfirmModalProps {
  isOpen: boolean;
  onConfirm: () => void;
  onCancel: () => void;
  closedEdgeCount: number;
  graphMode: 'campus' | 'f1';
}

export const ResetConfirmModal: React.FC<ResetConfirmModalProps> = ({
  isOpen,
  onConfirm,
  onCancel,
  closedEdgeCount,
  graphMode,
}) => {
  const confirmBtnRef = useRef<HTMLButtonElement | null>(null);

  useEffect(() => {
    if (isOpen) {
      confirmBtnRef.current?.focus();
    }
  }, [isOpen]);

  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (!isOpen) return;
      if (e.key === 'Escape') {
        onCancel();
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [isOpen, onCancel]);

  if (!isOpen) return null;

  return (
    <div
      className="modal-backdrop"
      role="presentation"
      onClick={onCancel}
    >
      <div
        className="modal-card reset-confirm-modal"
        role="dialog"
        aria-modal="true"
        aria-labelledby="reset-modal-title"
        aria-describedby="reset-modal-desc"
        onClick={(e) => e.stopPropagation()}
      >
        <div className="modal-header">
          <div className="modal-title-group">
            <span className="reset-modal-icon" aria-hidden="true">↺</span>
            <h3 id="reset-modal-title" className="modal-title">
              Reset Graph to Baseline?
            </h3>
          </div>
          <button
            type="button"
            className="modal-close-btn"
            onClick={onCancel}
            aria-label="Close dialog"
          >
            ✕
          </button>
        </div>

        <div className="modal-body" id="reset-modal-desc">
          <p className="reset-modal-summary">
            {closedEdgeCount > 0 ? (
              <>
                This will restore <strong>{closedEdgeCount} severed walkway{closedEdgeCount > 1 ? 's' : ''}</strong> and reset the route endpoints to default ({graphMode === 'campus' ? 'Main Gate → AB3' : 'Node 0 → Node 10'}).
              </>
            ) : (
              <>
                This will reset the route endpoints to their default baseline ({graphMode === 'campus' ? 'Main Gate → AB3' : 'Node 0 → Node 10'}).
              </>
            )}
          </p>
          <div className="reset-modal-info-box">
            <span className="info-box-icon" aria-hidden="true">ℹ️</span>
            <span>All C engine data structures, traversals, and operation counters will be re-initialized from the active fixture.</span>
          </div>
        </div>

        <div className="modal-footer">
          <button
            type="button"
            className="btn btn-secondary"
            onClick={onCancel}
            id="btn-cancel-reset"
          >
            Cancel
          </button>
          <button
            type="button"
            className="btn btn-primary"
            onClick={onConfirm}
            id="btn-confirm-reset"
            ref={confirmBtnRef}
          >
            Confirm Reset
          </button>
        </div>
      </div>
    </div>
  );
};
