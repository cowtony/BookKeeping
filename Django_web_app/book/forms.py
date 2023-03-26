from django import forms
from book.models import Transaction
from datetime import datetime

class TransactionForm(forms.ModelForm):
    class Meta:
        model = Transaction
        fields = ('DateTime', 'description',
                  'asset', 'expense', 'revenue', 'liability',)
        widgets = {
            'DateTime': forms.DateTimeInput(format='%Y-%m-%d %H:%M:%S'),
            # 'date': forms.NumberInput(attrs={'type': 'date'}),  # Example of Date picker
        }